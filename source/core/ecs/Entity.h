#pragma once

// #include "EntityManager.h"
#include "type.h"

namespace core::ecs
{

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

	template< typename ComponentType >
	ComponentType* add_component()
	{
		return EntityManager::get_instance()->add_component< ComponentType >( this );
	}

	template< typename ComponentType >
	ComponentType* get_component()
	{
		return EntityManager::get_instance()->get_component< ComponentType >( this );
	}

	void remove_component()
	{

	}

	const ComponentList& get_component_list()
	{
	}
	*/


	~Entity() = default;

	std::size_t get_id() const { return id_; }

}; // class Entity

} // namespace core::ecs
