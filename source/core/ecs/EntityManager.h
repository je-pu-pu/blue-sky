#pragma once

#include "Entity.h"
#include "Component.h"
#include "System.h"
#include "type.h"

#include <common/Singleton.h>

#include <vector>
#include <unordered_set>
#include <unordered_map>

namespace core::ecs
{

class EntityManager : public common::Singleton< EntityManager >
{
private:
	std::unordered_map< EntityId, Entity > entity_list_;
	std::unordered_map< ComponentTypeId, std::unordered_map< EntityId, Component > > component_list_;
	std::vector< System* > system_list_;

	EntityId next_entity_id_ = 1;

public:
	/**
	 * Entity ‚ð¶¬‚·‚é
	 */
	Entity* create_entity()
	{
		return & entity_list_.emplace( next_entity_id_, next_entity_id_ ).first->second;
	}

	/**
	 * Žw’è‚µ‚½ Entity ‚ð”jŠü‚·‚é
	 */
	void destroy_entity( EntityId id )
	{
		entity_list_.erase( id );
	}

	template< typename ComponentType >
	ComponentType* add_component( const Entity* e )
	{
		return & component_list_[ ComponentType::ID ][ e->get_id() ];
	}

	template< typename ComponentType >
	ComponentType* get_component( const Entity* e )
	{
		return & component_list_[ ComponentType::ID ][ e->get_id() ];
	}

	void update()
	{
		for ( auto s : system_list_ )
		{
			s->update();
		}
	}

}; // EntityManager

} // core::ecs
