#include "PlayerInventorySystem.h"

#include <blue_sky/ecs/EntityObject.h>
#include <blue_sky/ecs/component/PickupComponent.h>

#include <core/ecs/EntityManager.h>

namespace blue_sky::ecs
{

PlayerInventoryComponent* PlayerInventorySystem::find_inventory( core::ecs::EntityId id ) const
{
	core::ecs::Entity* entity = get_entity_manager()->find( id );

	return entity ? entity->get_component< PlayerInventoryComponent >() : nullptr;
}

void PlayerInventorySystem::play_sound( core::ecs::EntityId id, const char_t* name ) const
{
	const EntityObject object( get_entity_manager()->find( id ) );

	if ( object.is_valid() )
	{
		object.play_sound( sound_manager_, name );
	}
}

void PlayerInventorySystem::restart( core::ecs::EntityId id )
{
	PlayerInventoryComponent* inventory = find_inventory( id );

	if ( ! inventory )
	{
		return;
	}

	inventory->has_medal = false;

	for ( int n = 0; n < static_cast< int >( ItemType::MAX ); n++ )
	{
		inventory->item_count[ n ] = 0;
	}

	inventory->item_count[ static_cast< int >( ItemType::SCOPE ) ] = 1;
	inventory->selected_item_type = ItemType::NONE;
	inventory->stone_list.clear();
}

bool PlayerInventorySystem::pick_stone( core::ecs::EntityId id, const GameObjectRef& stone )
{
	PlayerInventoryComponent* inventory = find_inventory( id );

	if ( ! inventory )
	{
		return false;
	}

	inventory->item_count[ static_cast< int >( ItemType::STONE ) ]++;
	inventory->selected_item_type = ItemType::STONE;

	inventory->stone_list.push_back( stone );

	play_sound( id, "stone-get" );

	return true;
}

bool PlayerInventorySystem::pick_up( core::ecs::EntityId id, PickupKind kind )
{
	PlayerInventoryComponent* inventory = find_inventory( id );

	if ( ! inventory )
	{
		return false;
	}

	switch ( kind )
	{
	case PickupKind::MEDAL:
		inventory->has_medal = true;
		play_sound( id, "medal-get" );
		break;

	case PickupKind::ROCKET:
		inventory->item_count[ static_cast< int >( ItemType::ROCKET ) ]++;
		inventory->selected_item_type = ItemType::ROCKET;
		play_sound( id, "rocket-get" );
		break;

	case PickupKind::UMBRELLA:
		// 以前から、傘を拾ってもプレイヤーは何もしない（傘は消える）
		break;

	default:
		break;
	}

	return true;
}

void PlayerInventorySystem::select_prev_item( core::ecs::EntityId id )
{
	PlayerInventoryComponent* inventory = find_inventory( id );

	if ( ! inventory )
	{
		return;
	}

	for ( int type = static_cast< int >( inventory->selected_item_type ) - 1; type >= static_cast< int >( ItemType::NONE ); type-- )
	{
		if ( type == static_cast< int >( ItemType::NONE ) || inventory->item_count[ type ] )
		{
			inventory->selected_item_type = static_cast< ItemType >( type );
			play_sound( id, "click" );
			break;
		}
	}
}

void PlayerInventorySystem::select_next_item( core::ecs::EntityId id )
{
	PlayerInventoryComponent* inventory = find_inventory( id );

	if ( ! inventory )
	{
		return;
	}

	for ( int type = static_cast< int >( inventory->selected_item_type ) + 1; type < static_cast< int >( ItemType::MAX ); type++ )
	{
		if ( inventory->item_count[ type ] )
		{
			inventory->selected_item_type = static_cast< ItemType >( type );
			play_sound( id, "click" );
			break;
		}
	}
}

bool PlayerInventorySystem::use_rocket( core::ecs::EntityId id )
{
	PlayerInventoryComponent* inventory = find_inventory( id );

	if ( ! inventory || inventory->get_item_count( ItemType::ROCKET ) <= 0 )
	{
		return false;
	}

	inventory->item_count[ static_cast< int >( ItemType::ROCKET ) ]--;

	if ( inventory->item_count[ static_cast< int >( ItemType::ROCKET ) ] <= 0 )
	{
		inventory->selected_item_type = ItemType::NONE;
	}

	return true;
}

std::optional< GameObjectRef > PlayerInventorySystem::use_stone( core::ecs::EntityId id )
{
	PlayerInventoryComponent* inventory = find_inventory( id );

	if ( ! inventory || inventory->get_item_count( ItemType::STONE ) <= 0 || inventory->stone_list.empty() )
	{
		return std::nullopt;
	}

	GameObjectRef stone = inventory->stone_list.back();
	inventory->stone_list.pop_back();
	inventory->item_count[ static_cast< int >( ItemType::STONE ) ]--;

	if ( inventory->item_count[ static_cast< int >( ItemType::STONE ) ] <= 0 )
	{
		inventory->selected_item_type = ItemType::NONE;
	}

	return stone;
}

} // namespace blue_sky::ecs
