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
	EntityManager* manager_;		///< 自分を作った EntityManager（以前はシングルトンから引いていた）

public:
	Entity( std::size_t id, EntityManager* manager )
		: id_( id )
		, manager_( manager )
	{ }

	// 中身は EntityManager の定義の後（EntityManager.h の末尾）にある（EntityManager の完全な型が要るため）
	template< typename ComponentType >
	ComponentType* add_component() const;

	/// @todo 非 const ComponentType を返して良いか？
	template< typename ComponentType >
	ComponentType* get_component() const;

	template< typename ComponentType >
	void remove_component();

	/*
	const ComponentList& get_component_list()
	{
	}
	*/


	~Entity() = default;

	std::size_t get_id() const { return id_; }

}; // class Entity

} // namespace core::ecs
