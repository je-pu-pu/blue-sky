#pragma once

#include "type.h"
#include <boost/noncopyable.hpp>

/// @todo きれいにする
namespace core::graphics
{
	class GraphicsManager;
}

namespace core::ecs
{

class Entity;
class Component;

/**
 * システムの基底クラス
 *
 */
class BaseSystem : private boost::noncopyable
{
public:
	using GraphicsManager = core::graphics::GraphicsManager;

private:
	/// システムの実行優先度 ( priority_ が小さい順に実行される )
	int_t priority_ = 0;

protected:
	virtual bool has_component_type( ComponentTypeId ) const = 0;
	virtual void remove_entity_component( const Entity* ) = 0;

	GraphicsManager* get_graphics_manager();

public:
	BaseSystem() = default;
	virtual ~BaseSystem() = default;

	virtual void update() = 0;

	/**
	 * このシステムの実行優先度を取得する
	 * 
	 * @return システムの実行優先度 ( 数値が小さい順に実行される )
	 */
	int_t get_priority() const { return priority_; }

	/**
	 * このシステムの実行優先度を設定する
	 * 
	 * @param priority システムの実行優先度 ( 数値が小さい順に実行される )
	 */
	void set_priority( int_t priority = 0 ) { priority_ = priority; }

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

	virtual void add_entity_component_if_all_components_ready( const Entity* ) = 0;

}; // class System

} // namespace core::ecs
