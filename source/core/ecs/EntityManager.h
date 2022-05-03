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
	// Entity �̈ꗗ
	std::unordered_map< EntityId, Entity > entity_list_;

	// Component �̈ꗗ
	std::unordered_map< ComponentTypeId, EntityComponentMap > component_list_;

	// ( SystemTypeId => System �̎��� ) �̃}�b�v ( System �̎��Ԃ�ێ����� )
	std::unordered_map< SystemTypeId, std::unique_ptr< BaseSystem > > system_map_;

	// System �̈ꗗ ( ��Ɏ��s���ɕ���ł��鎖���ۏ؂���Ă��� )
	std::vector< BaseSystem* > system_list_;

	/// ���ɕt�^���� Entity ID
	EntityId next_entity_id_ = 0;

public:
	/**
	 * Entity �𐶐�����
	 */
	Entity* create_entity()
	{
		Entity* e = & entity_list_.emplace( next_entity_id_, next_entity_id_ ).first->second;

		next_entity_id_++;

		return e;
	}

	/**
	 * �w�肵�� Entity ��j������
	 */
	void destroy_entity( EntityId id )
	{
		entity_list_.erase( id );
	}

	/**
	 * Entity �� Component ��ǉ�����
	 * 
	 * @param e Entity
	 * @return �ǉ����� Component
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
	 * Entity ���� Component ����菜��
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
	 * EntityManager �ɃV�X�e����ǉ�����B
	 *
	 * ���łɃV�X�e�����ǉ�����Ă���ꍇ�́A�������Ȃ�
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

		// �����̑S Entity �ɑ΂��đ������s���A Entity �� System �̑���ΏۂƂȂ� Component ��S�Ď����Ă����ꍇ�ASystem �� Component �ւ̎Q�Ƃ�ǉ�����
		for ( auto& entity : entity_list_ )
		{
			system->add_entity_component_if_all_components_ready( & entity.second );
		}
	}

	/**
	 * EntityManager ���� System ����菜��
	 *
	 * EntityManager �� System �����݂��Ȃ��ꍇ�́A�������Ȃ�
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
	 * EntityManager �ɒǉ�����Ă���S�Ă� System �����s����
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
	 * �S�Ă� Entity, Component, System ���폜����
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
