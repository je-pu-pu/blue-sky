#include "CollisionEventSystem.h"

#include <core/ecs/EntityManager.h>

#include <utility>

namespace blue_sky::ecs
{

const CollisionTraitsComponent* CollisionEventSystem::find_traits( core::ecs::EntityId id ) const
{
	const core::ecs::EntityManager* entity_manager = get_entity_manager();
	const core::ecs::Entity* entity = entity_manager ? entity_manager->find( id ) : nullptr;

	return entity ? entity->get_component< CollisionTraitsComponent >() : nullptr;
}

CollisionEventSystem::Event CollisionEventSystem::classify( const Contact& contact ) const
{
	auto is_active = [ this ] ( core::ecs::EntityId id )
	{
		const CollisionTraitsComponent* traits = find_traits( id );
		return traits && ! traits->is_dead;
	};

	if ( ! contact.a && ! contact.b )
	{
		return { };
	}

	// 片方が地面（以前は userPointer が nullptr の側）
	if ( ! contact.a || ! contact.b )
	{
		const core::ecs::EntityId id = contact.a ? * contact.a : * contact.b;

		if ( ! is_active( id ) )
		{
			return { };
		}

		return { EventType::GROUND, id, id };
	}

	if ( ! is_active( * contact.a ) || ! is_active( * contact.b ) )
	{
		return { };
	}

	return { EventType::PAIR, * contact.a, * contact.b };
}

void CollisionEventSystem::dispatch( CollisionEventHandler& handler )
{
	// 渡した先で当たりが加わっても壊れないように、一覧を受け取ってから回す
	std::vector< Contact > contacts;
	contacts.swap( contact_list_ );

	for ( const auto& contact : contacts )
	{
		const Event event = classify( contact );

		switch ( event.type )
		{
		case EventType::GROUND:
			handler.on_collide_with_ground( event.a );
			break;

		case EventType::PAIR:
			handler.on_collide( event.a, event.b );
			break;

		default:
			break;
		}
	}

	// 一覧の確保を使い回す（渡した先で加わった組があれば残す）
	contacts.clear();

	if ( contact_list_.empty() )
	{
		contact_list_.swap( contacts );
	}
}

const CollisionTraitsComponent* CollisionEventSystem::find_traits( const core::ecs::EntityManager* entity_manager, std::optional< core::ecs::EntityId > id )
{

	if ( ! id || ! entity_manager )
	{
		return nullptr;
	}

	const core::ecs::Entity* entity = entity_manager->find( * id );

	return entity ? entity->get_component< CollisionTraitsComponent >() : nullptr;
}

bool CollisionEventSystem::is_vanished( const core::ecs::EntityManager* entity_manager, std::optional< core::ecs::EntityId > id )
{
	if ( ! id )
	{
		return false;
	}

	return ! entity_manager || ! entity_manager->is_alive( * id );
}

bool CollisionEventSystem::is_ray_target( const core::ecs::EntityManager* entity_manager, std::optional< core::ecs::EntityId > tag, bool include_soft_footing )
{
	// 以前の ClosestNotMe::addSingleResult と同じ順・同じ決まり（自分は PhysicsManager::ray_test_closest が除く）
	if ( is_vanished( entity_manager, tag ) )
	{
		return false;
	}

	if ( const auto* traits = find_traits( entity_manager, tag ) )
	{
		if ( traits->is_dead || traits->is_ghost )
		{
			return false;
		}

		if ( ! include_soft_footing && ! traits->is_hard )
		{
			return false;
		}
	}

	return true;
}

bool CollisionEventSystem::is_block( const core::ecs::EntityManager* entity_manager, std::optional< core::ecs::EntityId > tag )
{
	// 以前の WithoutMeContactResultCallback::is_ignored の逆（自分は PhysicsManager::box_contact_test が除く）
	if ( is_vanished( entity_manager, tag ) )
	{
		return false;
	}

	const auto* traits = find_traits( entity_manager, tag );

	return ! traits || traits->is_block;
}

} // namespace blue_sky::ecs
