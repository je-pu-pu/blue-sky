#pragma once

#include "Entity.h"
#include "Component.h"
#include "System.h"
#include "type.h"

#include <common/Singleton.h>

#include <vector>
#include <unordered_set>
#include <unordered_map>

#include <memory>

namespace core::ecs
{

class EntityManager : public common::Singleton< EntityManager >
{
public:
	using EntityComponentMap = std::unordered_map< EntityId, std::unique_ptr< Component > >;

private:
	std::unordered_map< EntityId, Entity > entity_list_;
	std::unordered_map< ComponentTypeId, EntityComponentMap > component_list_;
	std::unordered_map< SystemTypeId, std::unique_ptr< System > > system_list_;

	EntityId next_entity_id_ = 0;

public:
	/**
	 * Entity ‚ð¶¬‚·‚é
	 */
	Entity* create_entity()
	{
		Entity* e = & entity_list_.emplace( next_entity_id_, next_entity_id_ ).first->second;

		next_entity_id_++;

		return e;
	}

	/**
	 * Žw’è‚µ‚½ Entity ‚ð”jŠü‚·‚é
	 */
	void destroy_entity( EntityId id )
	{
		entity_list_.erase( id );
	}

	/**
	 * Entity ‚É Component ‚ð’Ç‰Á‚·‚é
	 * 
	 * @param e Entity
	 * @return ’Ç‰Á‚µ‚½ Component
	 */
	template< typename ComponentType >
	ComponentType* add_component( const Entity* e )
	{
		const auto component_type_id = typeid( ComponentType ).hash_code();
		auto i = component_list_.find( component_type_id );

		if ( i == component_list_.end() )
		{
			i = component_list_.emplace( component_type_id, EntityComponentMap() ).first;
		}

		auto i2 = i->second.find( e->get_id() );

		if ( i2 == i->second.end() )
		{
			auto c = new ComponentType();

			i->second.emplace( e->get_id(), c );

			return c;
		}

		return nullptr;
	}

	template< typename ComponentType >
	ComponentType* get_component( const Entity* e )
	{
		auto& list = component_list_[ typeid( ComponentType ).hash_code() ];

		auto i = list.find( e->get_id() );

		if ( i == list.end() )
		{
			return nullptr;
		}

		return static_cast< ComponentType* >( i->second );
	}

	template< typename SystemType >
	void add_system()
	{
		const auto id = typeid( SystemType ).hash_code();

		assert( system_list_.find( id ) == system_list_.end() );

		system_list_.emplace( id, new SystemType() );
	}

	template< typename SystemType >
	void remove_system()
	{
		const auto id = typeid( SystemType ).hash_code();

		auto i = system_list_.find( id );

		if ( i == system_list_.end() )
		{
			return;
		}
		
		// i->second;

		system_list_.erase( i );
	}

	void update()
	{
		for ( auto& s : system_list_ )
		{
			s.second->update();
		}
	}

}; // EntityManager

} // core::ecs
