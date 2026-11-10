#include "ObjectRenderDataSystem.h"

#include <blue_sky/ecs/component/AnimationComponent.h>
#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/graphics/Model.h>
#include <blue_sky/graphics/Line.h>
#include <blue_sky/ConstantBuffers.h>

#include <core/ecs/EntityManager.h>
#include <core/animation/AnimationPlayer.h>

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

	if ( animation )
	{
		animation_list_.insert( { added.first, animation } );
	}
}

void ObjectRenderDataSystem::on_remove_entity( core::ecs::EntityId id, ComponentTuple& )
{
	render_target_list_.erase( id );
	animation_list_.erase( id );
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
void ObjectRenderDataSystem::update_render_data()
{
	if ( ! graphics_manager_ )
	{
		return;
	}

	for ( const auto& target : get_render_targets() )
	{
		ObjectRenderComponent& render = * target.render;

		if ( ! render.is_visible || ! render.object_constant_buffer )
		{
			continue;
		}

		const graphics::Model* model = graphics_manager_->resolve_model( render.model );

		if ( ! model )
		{
			continue;
		}

		ObjectConstantBufferData shader_data;

		shader_data.world = make_world_matrix( target.transform->transform );
		shader_data.prev_world = advance_prev_world( render, shader_data.world );

		if ( model->get_line() )
		{
			shader_data.color = model->get_line()->get_color();
		}

		render.object_constant_buffer->update( & shader_data );

		const auto* animation = target.animation;

		if ( animation && animation->player && render.bone_constant_buffer )
		{
			core::BoneConstantBufferData bone_data;
			animation->player->calculate_bone_matrices( bone_data );
			render.bone_constant_buffer->update( & bone_data );
		}
	}
}

} // namespace blue_sky::ecs
