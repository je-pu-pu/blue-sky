#pragma once

// #include "EntityManager.h"
#include "type.h"

namespace core::ecs
{

class EntityManager;

/**
 * エンティティ
 *
 */
class Entity
{
private:
	EntityId id_;

public:
	Entity( std::size_t id )
		: id_( id )
	{ }

	/*
	static Entity* create()
	{
		return EntityManager::get_instance()->create_entity();
	}

	void destroy()
	{
		EntityManager::get_instance()->destroy_entity( this );
	}
	*/

	template< typename ComponentType >
	ComponentType* add_component() const
	{
		return EntityManager::get_instance()->add_component< ComponentType >( this );
	}

	/// @todo 非 const ComponentType を返して良いか？
	template< typename ComponentType >
	ComponentType* get_component() const
	{
		return EntityManager::get_instance()->get_component< ComponentType >( this );
	}

	/*
	template< typename ComponentType >
	const ComponentType* get_component() const
	{
		return EntityManager::get_instance()->get_component< ComponentType >( this );
	}
	*/

	template< typename ComponentType >
	void remove_component()
	{
		return EntityManager::get_instance()->remove_component< ComponentType >( this );
	}

	/*
	const ComponentList& get_component_list()
	{
	}
	*/


	~Entity() = default;

	std::size_t get_id() const { return id_; }

}; // class Entity

} // namespace core::ecs
