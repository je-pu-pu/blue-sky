#pragma once

#include "type.h"

namespace core::ecs
{

class Entity;
class Component;

/**
 * システムの基底クラス
 *
 */
class BaseSystem
{
public:

protected:
	virtual bool has_component_type( ComponentTypeId ) const = 0;
	virtual void add_entity_component_if_all_components_ready( const Entity* ) = 0;
	virtual void remove_entity_component( const Entity* ) = 0;

public:
	BaseSystem() = default;
	virtual ~BaseSystem() = default;

	virtual void update() = 0;

	/**
	 * Entity に Component が追加された時の処理
	 * 
	 * @param entity ComponentType の Component が追加された Entity
	 */
	template< typename ComponentType >
	void on_add_component( const Entity* entity )
	{
		// entity に追加された component が自身の System の操作対象に含まれているか調べ、含まれていなければ何もしない
		if ( ! has_component_type( ComponentType::get_type_id< ComponentType >() ) )
		{
			return;
		}

		// entity に component が追加された事により、entity に自身の System の操作対象となる全ての Component が揃っていれば、Component をシステムの操作対象として追加する
		add_entity_component_if_all_components_ready( entity );
	}

	/**
	 * Entity から Component が取り除かれた時の処理
	 * 
	 * @param entity ComponentType の Component が取り除かれた Entity
	 */
	template< typename ComponentType >
	void on_remove_component( const Entity* entity )
	{
		// entity から取り除かれた Component が自身の System の操作対象に含まれているか調べ、含まれていなければ何もしない
		if ( ! has_component_type( ComponentType::get_type_id< ComponentType >() ) )
		{
			return;
		}

		// entity から取り除かれた Component が自身の System の操作対象であれば、entity の Component をシステムから取り除く
		remove_entity_component( entity );
	}

}; // class System

} // namespace core::ecs
