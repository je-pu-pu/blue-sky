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
	virtual void check_entity( const Entity* ) = 0;

public:
	BaseSystem() = default;
	virtual ~BaseSystem() = default;

	virtual void update() = 0;

	/**
	 * Entity に Component が追加された時の処理
	 */
	template< typename ComponentType >
	void on_add_component( const Entity* entity, ComponentType* component )
	{
		// entity に追加された component が自身の System の操作対象に含まれているか調べ、含まれていなければ何もしない
		if ( ! has_component_type( ComponentType::get_type_id< ComponentType >() ) )
		{
			return;
		}

		// entity に component が追加された事により、entity に自身の System の操作対象となる全ての Component が揃ったかを調べ、
		check_entity( entity );
	}

}; // class System

} // namespace core::ecs
