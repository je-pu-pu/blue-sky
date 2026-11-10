#include "BalloonHoldSystem.h"

#include <blue_sky/ecs/EntityObject.h>
#include <blue_sky/ecs/Flicker.h>
#include <blue_sky/ecs/component/ObjectRenderComponent.h>
#include <blue_sky/ecs/system/PlayerBehaviorSystem.h>

#include <core/ecs/EntityManager.h>

#include <algorithm>

namespace blue_sky::ecs
{

void BalloonHoldSystem::update_balloons( float_t scene_elapsed_time )
{
	id_list_.clear();

	for ( const auto& i : get_component_list() )
	{
		id_list_.push_back( i.first );
	}

	std::sort( id_list_.begin(), id_list_.end() );

	for ( const core::ecs::EntityId id : id_list_ )
	{
		update_balloon( id, scene_elapsed_time );
	}
}

void BalloonHoldSystem::update_balloon( core::ecs::EntityId id, float_t scene_elapsed_time )
{
	core::ecs::Entity* entity = get_entity_manager()->find( id );
	auto* balloon = entity ? entity->get_component< BalloonHoldComponent >() : nullptr;

	if ( ! balloon )
	{
		return;
	}

	EntityObject object( entity );

	if ( ! object.is_valid() )
	{
		return;
	}

	// つかんだプレイヤー（消えていれば、持たれていないのと同じ扱いで割れる）
	core::ecs::Entity* holder_entity = balloon->holder_id ? get_entity_manager()->find( * balloon->holder_id ) : nullptr;
	const PlayerBehaviorComponent* holder = holder_entity ? holder_entity->get_component< PlayerBehaviorComponent >() : nullptr;

	if ( balloon->is_held() )
	{
		// 今持っている風船がこれか（後から別の風船をつかむと、前の風船は持たれていない）
		if ( holder && holder->balloon == id )
		{
			object.set_velocity( core::Vector( 0, 0, 0 ) );
			object.set_location( PlayerBehaviorSystem::get_balloon_hold_location( * holder_entity ) );
			object.set_direction_degree( 0 );

			// 風船の動作が終わると割れる
			if ( holder->action_mode != PlayerBehaviorComponent::ActionMode::BALLOON )
			{
				object.kill();
			}
		}
		else
		{
			object.kill();
		}
	}
	else
	{
		object.update_location_by_flicker( object.get_start_location(), BalloonHoldComponent::FlickerScale, scene_elapsed_time );
		object.set_direction_degree( object.get_direction_degree() + BalloonHoldComponent::SpinDegreePerFrame );
	}

	// つかんだ物体の風船の動作が終わりかけの間は点滅する（以前の Balloon::is_visible_with_player。以前は描く前に毎回決めていた）
	if ( auto* render = entity->get_component< ObjectRenderComponent >() )
	{
		const bool is_visible_with_holder = ! holder || ! holder->is_action_pre_finish || flicker::is_visible_in_blink( scene_elapsed_time, BalloonHoldComponent::BlinkCount );
		render->is_hidden = ! is_visible_with_holder;
	}
}

bool BalloonHoldSystem::grab( core::ecs::EntityId balloon_id, core::ecs::EntityId holder_id )
{
	core::ecs::Entity* entity = get_entity_manager()->find( balloon_id );
	auto* balloon = entity ? entity->get_component< BalloonHoldComponent >() : nullptr;

	if ( ! balloon || balloon->is_held() )
	{
		return false;
	}

	balloon->holder_id = holder_id;

	return true;
}

void BalloonHoldSystem::burst( core::ecs::EntityId id )
{
	core::ecs::Entity* entity = get_entity_manager()->find( id );

	if ( ! entity || ! entity->get_component< BalloonHoldComponent >() )
	{
		return;
	}

	EntityObject object( entity );

	object.kill();
	object.play_sound( sound_manager_, "balloon-burst" );
}

bool BalloonHoldSystem::is_balloon( core::ecs::EntityId id ) const
{
	const core::ecs::Entity* entity = get_entity_manager()->find( id );

	return entity && entity->get_component< BalloonHoldComponent >();
}

} // namespace blue_sky::ecs
