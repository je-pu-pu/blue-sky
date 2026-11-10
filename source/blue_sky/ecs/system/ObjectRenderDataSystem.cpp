#include "ObjectRenderDataSystem.h"

#include <blue_sky/ecs/component/AnimationComponent.h>
#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/graphics/Model.h>
#include <blue_sky/graphics/Line.h>
#include <blue_sky/ConstantBuffers.h>

#include <core/ecs/EntityManager.h>
#include <core/animation/AnimationPlayer.h>

#include <algorithm>

namespace blue_sky::ecs
{

void ObjectRenderDataSystem::add_entity_component_if_all_components_ready( const core::ecs::Entity* entity )
{
	System::add_entity_component_if_all_components_ready( entity );

	is_render_target_list_dirty_ = true;
}

bool ObjectRenderDataSystem::is_watched_component_type( core::ecs::ComponentTypeId id ) const
{
	return id == core::ecs::get_component_type_id< AnimationComponent >();
}

/**
 * 描く対象の一覧とアニメーションを持つものの一覧を作り直す（対象の増減・アニメーションの部品の付け外しの後だけ）
 */
void ObjectRenderDataSystem::rebuild_render_targets_if_dirty() const
{
	if ( ! is_render_target_list_dirty_ )
	{
		return;
	}

	render_target_list_.clear();
	render_target_list_.reserve( get_component_list().size() );

	for ( const auto& i : get_component_list() )
	{
		const core::ecs::Entity* entity = get_entity_manager() ? get_entity_manager()->find( i.first ) : nullptr;
		const AnimationComponent* animation = entity ? entity->get_component< AnimationComponent >() : nullptr;

		render_target_list_.push_back( { i.first, std::get< ObjectRenderComponent* >( i.second ), std::get< core::ecs::TransformComponent* >( i.second ), animation } );
	}

	std::sort( render_target_list_.begin(), render_target_list_.end(), [] ( const RenderTarget& a, const RenderTarget& b ) { return a.id < b.id; } );

	animation_list_.clear();

	for ( const auto& target : render_target_list_ )
	{
		if ( target.animation )
		{
			animation_list_.push_back( target.animation );
		}
	}

	is_render_target_list_dirty_ = false;
}

const ObjectRenderDataSystem::RenderTargetList& ObjectRenderDataSystem::get_render_targets() const
{
	rebuild_render_targets_if_dirty();

	return render_target_list_;
}

const AnimationComponent* ObjectRenderDataSystem::get_animation( core::ecs::EntityId id ) const
{
	const auto& targets = get_render_targets();
	auto i = std::lower_bound( targets.begin(), targets.end(), id, [] ( const RenderTarget& t, core::ecs::EntityId id ) { return t.id < id; } );

	return ( i != targets.end() && i->id == id ) ? i->animation : nullptr;
}

/// アニメーションを持つ物体だけを回す（以前は描く対象すべてについて、Entity とアニメーションの部品を引いていた）
void ObjectRenderDataSystem::update_animations()
{
	rebuild_render_targets_if_dirty();

	for ( const auto* animation : animation_list_ )
	{
		if ( animation->player )
		{
			animation->player->update();
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
