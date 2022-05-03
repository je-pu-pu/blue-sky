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

#include <iostream>

namespace core::ecs
{

class EntityManager : public common::Singleton< EntityManager >
{
public:
	using EntityComponentMap = std::unordered_map< EntityId, std::unique_ptr< Component > >;

private:
	// Entity の一覧
	std::unordered_map< EntityId, Entity > entity_list_;

	// Component の一覧
	std::unordered_map< ComponentTypeId, EntityComponentMap > component_list_;

	// ( SystemTypeId => System の実態 ) のマップ ( System の実態を保持する )
	std::unordered_map< SystemTypeId, std::unique_ptr< BaseSystem > > system_map_;

	// System の一覧 ( 常に実行順に並んでいる事が保証されている )
	std::vector< BaseSystem* > system_list_;

	/// 次に付与する Entity ID
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

			for ( auto& s : system_list_ )
			{
				s->on_add_component< ComponentType >( e );
			}

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

		for ( auto& s : system_list_ )
		{
			s->on_remove_component< ComponentType >( e );
		}
	}

	/**
	 * EntityManager にシステムを追加する。
	 *
	 * すでにシステムが追加されている場合は、何もしない
	 */
	template< typename SystemType >
	void add_system( int priority = 0 )
	{
		const auto id = typeid( SystemType ).hash_code();

		if ( system_map_.find( id ) != system_map_.end() )
		{
			return;
		}

		auto system = new SystemType();
		system->set_priority( priority );

		system_map_.emplace( id, system );
		
		system_list_.push_back( system );
		std::sort( system_list_.begin(), system_list_.end(), [] ( const BaseSystem* a, const BaseSystem* b ) { return a->get_priority() < b->get_priority(); } );

		// 既存の全 Entity に対して走査を行い、 Entity が System の操作対象となる Component を全て持っていた場合、System に Component への参照を追加する
		for ( auto& entity : entity_list_ )
		{
			system->add_entity_component_if_all_components_ready( & entity.second );
		}
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

		auto i = system_map_.find( id );

		if ( i == system_map_.end() )
		{
			return;
		}

		system_list_.erase( std::remove( system_list_.begin(), system_list_.end(), i->second.get() ) );

		system_map_.erase( i );	
	}

	/**
	 * EntityManager に追加されている全ての System を実行する
	 *
	 */
	void update()
	{
		std::cout << "EntityManager::update()" << std::endl;

		for ( auto& s : system_list_ )
		{
			s->update();
		}
	}

	/**
	 * 全ての Entity, Component, System を削除する
	 */
	void clear()
	{
		entity_list_.clear();
		component_list_.clear();
		system_map_.clear();
		system_list_.clear();

		// next_entity_id_ = 0;
	}

}; // EntityManager

} // core::ecs
