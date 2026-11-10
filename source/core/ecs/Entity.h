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
	// const の正しさ : 足す・外すは非 const。const の Entity からは const のコンポーネントだけを得る
	// （以前は const のメソッドが非 const のコンポーネントを返し、足すのも const だった）
	template< typename ComponentType >
	ComponentType* add_component();

	template< typename ComponentType >
	ComponentType* get_component();

	template< typename ComponentType >
	const ComponentType* get_component() const;

	template< typename ComponentType >
	void remove_component();


	~Entity() = default;

	std::size_t get_id() const { return id_; }

	/// 自分を作った EntityManager（部品の組み合わせで振る舞いを決める所が、相手の System を引くのに使う）
	EntityManager* get_entity_manager() const { return manager_; }

}; // class Entity

} // namespace core::ecs
