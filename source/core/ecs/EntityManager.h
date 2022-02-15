#pragma once

#include "Entity.h"
#include "Component.h"
#include "BaseSystem.h"
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
	std::unordered_map< SystemTypeId, std::unique_ptr< BaseSystem > > system_list_;

	EntityId next_entity_id_ = 0;

public:
	/**
	 * Entity を生成する
	 */
	Entity* create_entity()
	{
		Entity* e = & entity_list_.emplace( next_entity_id_, next_entity_id_ ).first->second;

		next_entity_id_++;

		return e;
	}

	/**
	 * 指定した Entity を破棄する
	 */
	void destroy_entity( EntityId id )
	{
		entity_list_.erase( id );
	}

	/**
	 * Entity に Component を追加する
	 * 
	 * @param e Entity
	 * @return 追加した Component
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

		return static_cast< ComponentType* >( i->second.get() );
	}

	/**
	 * Entity から Component を取り除く
	 * 
	 * @param e Entity
	 */
	template< typename ComponentType >
	void remove_component( const Entity* e )
	{
		const auto component_type_id = typeid( ComponentType ).hash_code();
		auto i = component_list_.find( component_type_id );

		if ( i == component_list_.end() )
		{
			return;
		}

		i->second.erase( e->get_id() );
	}

	/**
	 * EntityManager にシステムを追加する。
	 *
	 * すでにシステムが追加されている場合は、何もしない
	 */
	template< typename SystemType >
	void add_system()
	{
		const auto id = typeid( SystemType ).hash_code();

		if ( system_list_.find( id ) != system_list_.end() )
		{
			return;
		}

		system_list_.emplace( id, new SystemType() );
	}

	/**
	 * EntityManager から System を取り除く
	 *
	 * EntityManager に System が存在しない場合は、何もしない
	 */
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

	/**
	 * EntityManager に追加されている全ての System を実行する
	 *
	 * @todo System の実行順を制御できるようにする
	 */
	void update()
	{
		for ( auto& s : system_list_ )
		{
			s.second->update();
		}
	}

}; // EntityManager

} // core::ecs
