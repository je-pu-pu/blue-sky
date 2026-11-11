#include "ObjectRenderDataSystem.h"

#include <blue_sky/ecs/component/AnimationComponent.h>
#include <blue_sky/graphics/ResourceResolver.h>
#include <blue_sky/graphics/Model.h>
#include <blue_sky/graphics/Line.h>
#include <blue_sky/ConstantBuffers.h>

#include <core/ecs/EntityManager.h>
#include <core/animation/AnimationPlayer.h>

#include <algorithm>

namespace blue_sky::ecs
{

/**
 * 対象に入ったら、描く対象の一覧に足す（番号は増える一方なので、たいていは末尾に足すだけ）
 *
 * アニメーションはここで一度だけ引く（後から付け外しされたら on_watched_component_changed で直す）
 */
void ObjectRenderDataSystem::add_entity_component_if_all_components_ready( const core::ecs::Entity* entity )
{
	const auto count = debug_entity_count();

	System::add_entity_component_if_all_components_ready( entity );

	if ( debug_entity_count() == count )
	{
		return;	// 対象に入らなかった（部品が揃っていない・既に対象）
	}

	// 対象の一覧（System）の末尾が今入った Entity
	const auto& added = get_component_list().back();
	const AnimationComponent* animation = entity->get_component< AnimationComponent >();

	render_target_list_.insert( { added.first, std::get< ObjectRenderComponent* >( added.second ), std::get< core::ecs::TransformComponent* >( added.second ), animation } );

	// このフレームのモデルを引いた後に入ったら、次の resolve_models で全部を引き直す
	// （ここでは引かない。部品のハンドルは、部品が揃った後で設定されることがある（ObjectFactory など））
	models_resolved_ = false;

	// 描く対象の並びが変わったので、このフレームのまとめは使わない（このフレームの残りは 1 個ずつ描く）
	instances_ready_ = false;

	if ( animation )
	{
		animation_list_.insert( { added.first, animation } );
	}
}

void ObjectRenderDataSystem::on_remove_entity( core::ecs::EntityId id, ComponentTuple& )
{
	render_target_list_.erase( id );
	animation_list_.erase( id );

	// 描く対象の並びが変わる（次に読むときに詰める）ので、このフレームのまとめは使わない
	instances_ready_ = false;
}

bool ObjectRenderDataSystem::is_watched_component_type( core::ecs::ComponentTypeId id ) const
{
	return id == core::ecs::get_component_type_id< AnimationComponent >();
}

/// アニメーションの部品が付いた・外れる。描く対象なら、その 1 つだけを直す（外れるときは部品がまだ生きているので、is_added で見分ける）
void ObjectRenderDataSystem::on_watched_component_changed( const core::ecs::Entity* entity, bool is_added )
{
	const auto id = static_cast< core::ecs::EntityId >( entity->get_id() );
	RenderTarget* target = render_target_list_.find( id );

	if ( ! target )
	{
		return;	// 描く対象でない（後で対象に入るときに引く）
	}

	if ( is_added )
	{
		target->animation = entity->get_component< AnimationComponent >();
		animation_list_.insert( { id, target->animation } );
	}
	else
	{
		target->animation = nullptr;
		animation_list_.erase( id );
	}
}

const ObjectRenderDataSystem::RenderTargetList& ObjectRenderDataSystem::get_render_targets() const
{
	return render_target_list_.get();
}

const AnimationComponent* ObjectRenderDataSystem::get_animation( core::ecs::EntityId id ) const
{
	const RenderTarget* target = render_target_list_.find( id );

	return target ? target->animation : nullptr;
}

/// アニメーションを持つ物体だけを回す（以前は描く対象すべてについて、Entity とアニメーションの部品を引いていた）
void ObjectRenderDataSystem::update_animations()
{
	for ( const auto& target : animation_list_.get() )
	{
		if ( target.animation->player )
		{
			target.animation->player->update();
		}
	}
}

core::Matrix ObjectRenderDataSystem::make_world_matrix( const core::Transform& transform )
{
	core::Matrix world;

	world.set_rotation_quaternion( transform.get_rotation() );
	world *= core::Matrix().set_translation( transform.get_position().x(), transform.get_position().y(), transform.get_position().z() );

	return world;
}

core::Matrix ObjectRenderDataSystem::advance_prev_world( ObjectRenderComponent& render, const core::Matrix& world )
{
	const core::Matrix prev = render.prev_world_valid ? render.prev_world : world;

	render.prev_world = world;
	render.prev_world_valid = true;

	return prev;
}

/**
 * 描く対象の定数バッファを書く（以前の ActiveObject::update_render_data と同じ中身）
 *
 * 描かない物体（is_visible が false）・定数バッファの無い物体・モデルの無い物体は書かない
 * （前フレームの world も進めない。以前と同じ）
 */
void ObjectRenderDataSystem::resolve_models() const
{
	if ( models_resolved_ )
	{
		return;
	}

	for ( const auto& target : get_render_targets() )
	{
		// GraphicsManager はモデルを blue_sky のモデルとして入れている（ResourceResolver::resolve_model と同じ）
		target.model = static_cast< const graphics::Model* >( target.render->model.get() );
	}

	models_resolved_ = true;
}

void ObjectRenderDataSystem::forget_resolved_models()
{
	models_resolved_ = false;

	// 描く範囲もこのフレームだけ（次のフレームで受け取らなければ、全部を描く）
	culling_requested_ = false;
	culling_applied_ = false;

	// まとめもこのフレームだけ（次のフレームの update_render_data で作り直す）
	instances_ready_ = false;

	for ( auto& list : draw_lists_ )
	{
		list.built = false;
	}

#ifdef _DEBUG
	for ( const auto& target : get_render_targets() )
	{
		target.model = nullptr;
	}
#endif
}

void ObjectRenderDataSystem::set_culling( const core::graphics::Frustum& camera, const core::graphics::Frustum* shadow_cascades, int cascade_count )
{
	camera_frustum_ = camera;
	shadow_cascade_count_ = shadow_cascades ? std::min( cascade_count, MaxShadowCascades ) : 0;

	for ( int n = 0; n < shadow_cascade_count_; n++ )
	{
		shadow_frustums_[ n ] = shadow_cascades[ n ];
	}

	culling_requested_ = true;
}

std::uint8_t ObjectRenderDataSystem::calculate_visibility( const RenderTarget& target, const core::Matrix& world )
{
	core::Vector3 local_center;
	float_t radius = 0.f;

	if ( ! target.model || ! target.model->get_cull_sphere( local_center, radius ) )
	{
		culling_stats_.always_visible++;
		return VisibleAll;
	}

	// 囲む球の中心を世界へ（world は回転と移動だけ。拡大縮小は使わないので半径はそのまま）
	const core::Vector center = core::Vector( local_center.x(), local_center.y(), local_center.z(), 1.f ) * world;

	std::uint8_t visibility = 0;

	if ( camera_frustum_.intersects_sphere( center, radius ) )
	{
		visibility |= VisibleInCamera;
		culling_stats_.in_camera++;
	}

	for ( int n = 0; n < shadow_cascade_count_; n++ )
	{
		if ( shadow_frustums_[ n ].intersects_sphere( center, radius ) )
		{
			visibility |= visible_in_shadow_cascade( n );
			culling_stats_.in_shadow_cascade[ n ]++;
		}
	}

	return visibility;
}

void ObjectRenderDataSystem::update_render_data()
{
	if ( ! resources_ )
	{
		return;
	}

	resolve_models();

	if ( culling_requested_ )
	{
		culling_stats_ = CullingStats();
	}

	instances_.clear();

	for ( auto& list : draw_lists_ )
	{
		list.built = false;
	}

	for ( const auto& target : get_render_targets() )
	{
		// 先に全部の段に入れておく（下で書かない物も、前のフレームの結果を持ち越さない）
		target.visibility = VisibleAll;
		target.instance = NoInstance;

		ObjectRenderComponent& render = * target.render;

		if ( ! render.should_write_constants() || ! render.object_constant_buffer )
		{
			continue;
		}

		const graphics::Model* model = target.model;

		if ( ! model )
		{
			continue;
		}

		ObjectConstantBufferData shader_data;

		shader_data.world = make_world_matrix( target.transform->transform );
		shader_data.prev_world = advance_prev_world( render, shader_data.world );

		if ( culling_requested_ )
		{
			culling_stats_.targets++;
			target.visibility = calculate_visibility( target, shader_data.world );

			// どの段にも入らない物は描かないので、定数バッファを書かない（前フレームの world は上で進めた。次に入ったときに正しい値を書く）
			if ( target.visibility == 0 )
			{
				continue;
			}
		}

		if ( model->get_line() )
		{
			shader_data.color = model->get_line()->get_color();
		}

		render.object_constant_buffer->update( & shader_data );

		// まとめて描くときに、インスタンスの入れ物へ写す（同じ中身）
		if ( instancing_enabled_ )
		{
			target.instance = static_cast< std::uint32_t >( instances_.size() );
			instances_.push_back( shader_data );
		}

		const auto* animation = target.animation;

		if ( animation && animation->player && render.bone_constant_buffer )
		{
			core::BoneConstantBufferData bone_data;
			animation->player->calculate_bone_matrices( bone_data );
			render.bone_constant_buffer->update( & bone_data );
		}
	}

	culling_applied_ = culling_requested_;
	instances_ready_ = instancing_enabled_;
}

bool ObjectRenderDataSystem::is_drawn_in_pass( const RenderTarget& target, int pass ) const
{
	const graphics::Model* model = target.model;

	// 定数を求めていない物（描かない・範囲の外・モデルが無い）は、どの段にも描かない
	if ( ! model || target.instance == NoInstance )
	{
		return false;
	}

	const ObjectRenderComponent& render = * target.render;

	if ( pass == MeshPass )
	{
		return is_visible( target, VisibleInCamera ) && render.should_draw_mesh() && model->get_mesh();
	}

	if ( pass == LinePass )
	{
		return is_visible( target, VisibleInCamera ) && render.should_draw_line() && model->get_line();
	}

	// 影の段（スキンメッシュは別に、今までどおり 1 個ずつ描く）
	if ( model->is_skin_mesh() )
	{
		return false;
	}

	if ( pass >= shadow_mesh_pass( 0 ) && pass < shadow_mesh_pass( MaxShadowCascades ) )
	{
		const int level = pass - shadow_mesh_pass( 0 );

		return is_visible( target, visible_in_shadow_cascade( level ) ) && render.should_draw_mesh() && model->get_mesh();
	}

	if ( pass >= shadow_line_pass( 0 ) && pass < shadow_line_pass( MaxShadowCascades ) )
	{
		const int level = pass - shadow_line_pass( 0 );

		return is_visible( target, visible_in_shadow_cascade( level ) ) && render.should_draw_line() && model->get_line() && model->get_line()->is_cast_shadow();
	}

	return false;
}

bool ObjectRenderDataSystem::can_batch_in_pass( const RenderTarget& target, int pass )
{
	if ( pass == MeshPass )
	{
		// ボーンの定数（アニメーション）は物体ごとに違うので、まとめない。シェーダーがまとめて描けないモデルもまとめない
		return ! target.render->bone_constant_buffer && target.model->can_render_object_instances();
	}

	// 線・影の段（スキンメッシュでない物）は、どれもまとめる
	return true;
}

void ObjectRenderDataSystem::build_draw_list( int pass, DrawList& list ) const
{
	list.items.clear();
	list.batches.clear();
	list.instance_indices.clear();

	// まとめごとに、入る物の定数の位置を、並びの順に集める（まとめ・定数の位置。後でまとめごとに並べ直す）
	auto& members = scratch_members_;
	members.clear();

	// モデル → まとめ（モデルの種類は少ないので並びで持つ）。同じモデルが続くことが多いので、直前のモデルを先に見る
	auto& batch_of_model = scratch_batch_of_model_;
	batch_of_model.clear();

	const graphics::Model* last_model = nullptr;
	std::uint32_t last_batch = NoBatch;

	const auto& targets = get_render_targets();

	for ( std::uint32_t n = 0; n < targets.size(); n++ )
	{
		const RenderTarget& target = targets[ n ];

		if ( ! is_drawn_in_pass( target, pass ) )
		{
			continue;
		}

		if ( ! can_batch_in_pass( target, pass ) )
		{
			list.items.push_back( { n, NoBatch } );
			continue;
		}

		std::uint32_t batch = NoBatch;

		if ( target.model == last_model )
		{
			batch = last_batch;
		}
		else
		{
			for ( const auto& [ model, b ] : batch_of_model )
			{
				if ( model == target.model )
				{
					batch = b;
					break;
				}
			}

			if ( batch == NoBatch )
			{
				// 初めて出てきたモデル : この位置でまとめて描く
				batch = static_cast< std::uint32_t >( list.batches.size() );
				list.batches.push_back( { target.model, 0, 0 } );
				list.items.push_back( { n, batch } );
				batch_of_model.emplace_back( target.model, batch );
			}

			last_model = target.model;
			last_batch = batch;
		}

		list.batches[ batch ].count++;
		members.emplace_back( batch, target.instance );
	}

	// まとめごとの先頭を決め、入る物の定数の位置を、まとめの中は並びの順に詰める
	std::uint32_t first = 0;

	for ( auto& batch : list.batches )
	{
		batch.first = first;
		first += batch.count;
		batch.count = 0;
	}

	list.instance_indices.resize( first );

	for ( const auto& [ b, instance ] : members )
	{
		InstanceBatch& batch = list.batches[ b ];
		list.instance_indices[ batch.first + batch.count++ ] = instance;
	}

	list.built = true;
}

const ObjectRenderDataSystem::DrawList& ObjectRenderDataSystem::get_draw_list( int pass ) const
{
	DrawList& list = draw_lists_[ pass ];

	if ( ! list.built )
	{
		build_draw_list( pass, list );
	}

	return list;
}

} // namespace blue_sky::ecs
