#include "SwitchSystem.h"

#include <blue_sky/ecs/EntityObject.h>
#include <blue_sky/ecs/component/AnimationComponent.h>
#include <blue_sky/ecs/component/ContactCacheComponent.h>

#include <core/animation/AnimationPlayer.h>
#include <core/ecs/EntityManager.h>

namespace blue_sky::ecs
{

SwitchSystem::SwitchSystem( core::sound::SoundManager* sound_manager )
	: sound_manager_( sound_manager )
{

}

SwitchComponent* SwitchSystem::find_switch( EntityId id )
{
	core::ecs::Entity* entity = get_entity_manager() ? get_entity_manager()->find( id ) : nullptr;

	return entity ? entity->get_component< SwitchComponent >() : nullptr;
}

void SwitchSystem::play_feedback( EntityId id, const char_t* animation, bool loop, const char_t* sound )
{
	core::ecs::Entity* entity = get_entity_manager()->find( id );

	if ( ! entity || ! entity->get_component< SwitchComponent >()->has_feedback )
	{
		return;
	}

	// 以前の ActiveObject::play_animation( name, false, loop )（アニメーションを持たないモデルなら何もしない）
	if ( auto* a = entity->get_component< AnimationComponent >(); a && a->player )
	{
		a->player->play( animation, false, loop );
	}

	if ( sound )
	{
		EntityObject( entity ).play_sound( sound_manager_, sound );
	}
}

void SwitchSystem::call_event( EntityId id, const string_t& name )
{
	SwitchComponent* s = find_switch( id );

	if ( ! s )
	{
		return;
	}

	auto i = s->event_handler_map.find( name );

	if ( i == s->event_handler_map.end() )
	{
		return;
	}

	for ( auto j = i->second.begin(); j != i->second.end(); ++j )
	{
		( *j )();
	}
}

void SwitchSystem::on_touch( EntityId id, EntityId toucher_id, const core::Vector& velocity )
{
	core::ecs::Entity* entity = get_entity_manager() ? get_entity_manager()->find( id ) : nullptr;
	auto* cache = entity ? entity->get_component< ContactCacheComponent >() : nullptr;

	if ( ! cache || ! entity->get_component< SwitchComponent >() )
	{
		return;
	}

	if ( velocity.length() >= 0.5f && ! cache->cache.is_cached( toucher_id ) )
	{
		do_switch( id );
	}

	// do_switch のイベントで物体が加わっても部品の場所は動かない（StablePool）が、念のため引き直す
	if ( entity = get_entity_manager()->find( id ); entity )
	{
		if ( auto* c = entity->get_component< ContactCacheComponent >() )
		{
			c->cache.cache( toucher_id, ContactCacheComponent::LifeTime );
		}
	}
}

bool SwitchSystem::do_switch( EntityId id )
{
	SwitchComponent* s = find_switch( id );

	if ( ! s || s->state == State::BROKEN )
	{
		return false;
	}

	if ( s->state == State::ON )
	{
		if ( turn_off( id ) )
		{
			call_event( id, "turn-off" );
			return true;
		}
	}
	else if ( s->state == State::OFF )
	{
		if ( turn_on( id ) )
		{
			call_event( id, "turn-on" );
			return true;
		}
	}

	return false;
}

bool SwitchSystem::do_break( EntityId id )
{
	SwitchComponent* s = find_switch( id );

	if ( ! s || s->state == State::BROKEN )
	{
		return false;
	}

	s->state = State::BROKEN;

	play_feedback( id, "OnOff", true, nullptr );

	return true;
}

bool SwitchSystem::turn_on( EntityId id )
{
	SwitchComponent* s = find_switch( id );

	if ( ! s || s->state == State::ON )
	{
		return false;
	}

	s->state = State::ON;

	play_feedback( id, "TurnOn", false, "switch-on" );

	return true;
}

bool SwitchSystem::turn_off( EntityId id )
{
	SwitchComponent* s = find_switch( id );

	if ( ! s || s->state == State::OFF )
	{
		return false;
	}

	s->state = State::OFF;

	play_feedback( id, "TurnOff", false, "switch-off" );

	return true;
}

bool SwitchSystem::add_event_handler( EntityId id, const string_t& name, const EventHandler& handler )
{
	SwitchComponent* s = find_switch( id );

	if ( ! s )
	{
		return false;
	}

	s->event_handler_map[ name ].push_back( handler );

	return true;
}

std::optional< SwitchSystem::State > SwitchSystem::get_state( EntityId id ) const
{
	const core::ecs::Entity* entity = get_entity_manager() ? get_entity_manager()->find( id ) : nullptr;
	const auto* s = entity ? entity->get_component< SwitchComponent >() : nullptr;

	if ( ! s )
	{
		return std::nullopt;
	}

	return s->state;
}

} // namespace blue_sky::ecs
