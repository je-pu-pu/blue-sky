#pragma once

#include "type.h"
#include <boost/noncopyable.hpp>
#include <typeinfo>

namespace core::ecs
{

class EntityManager;

class Entity;
class Component;

/**
 * システムの基底クラス
 *
 */
class BaseSystem : private boost::noncopyable
{
private:
	/// システムの実行優先度 ( priority_ が小さい順に実行される )
	int_t priority_ = 0;

	/// 自分を持つ EntityManager（add_system が設定する。ほかの System を引くのに使う）
	EntityManager* entity_manager_ = nullptr;

protected:
	virtual bool has_component_type( ComponentTypeId ) const = 0;
	virtual void remove_entity_component( const Entity* ) = 0;

	/**
	 * 対象の部品（System< ... > の型）ではないが、付け外しを知りたい部品の型か
	 *
	 * 持っていても持っていなくても対象になる部品（例 : 描く物体のうちアニメーションを持つものだけが持つ部品）を、
	 * 毎フレーム引き直さずに済ませるために使う。既定は無し
	 */
	virtual bool is_watched_component_type( ComponentTypeId ) const { return false; }

	/**
	 * 付け外しを知りたい部品（is_watched_component_type）が entity に付いた・外れる
	 *
	 * 外れるときは、部品がまだ生きているうちに呼ぶ。entity がこの System の対象かは問わない
	 */
	virtual void on_watched_component_changed( const Entity* ) { }

public:
	BaseSystem() = default;
	virtual ~BaseSystem() = default;

	virtual void update() = 0;

	void set_entity_manager( EntityManager* m ) { entity_manager_ = m; }
	EntityManager* get_entity_manager() const { return entity_manager_; }

	/**
	 * 描く System か（RenderSystem など）
	 *
	 * 描く System は EntityManager::update() では動かさず、EntityManager::render()（シーンの render() から呼ぶ）で動かす。
	 * 以前は更新の中で描いていて、フレームの頭の画面のクリアに消され、ECS のシーンが真っ黒になっていた
	 */
	virtual bool is_render_system() const { return false; }

	/**
	 * このシステムの実行優先度を取得する
	 *
	 * @return システムの実行優先度 ( 数値が小さい順に実行される )
	 */
	int_t get_priority() const { return priority_; }

	/**
	 * このシステムの名前を取得する (デバッグ用)
	 */
	virtual const char* get_name() const { return typeid( *this ).name(); }

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
		const auto component_type_id = get_component_type_id< ComponentType >();

		if ( ! has_component_type( component_type_id ) )
		{
			if ( is_watched_component_type( component_type_id ) )
			{
				on_watched_component_changed( entity );
			}

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
		const auto component_type_id = get_component_type_id< ComponentType >();

		if ( ! has_component_type( component_type_id ) )
		{
			if ( is_watched_component_type( component_type_id ) )
			{
				on_watched_component_changed( entity );
			}

			return;
		}

		// entity から取り除かれた Component が自身の System の操作対象であれば、entity の Component をシステムから取り除く
		remove_entity_component( entity );
	}

	/**
	 * Entity が破棄される時の処理（EntityManager::destroy_entity が、コンポーネントを外す前に呼ぶ）
	 *
	 * @param entity 破棄される Entity（この System の対象でなければ何もしない）
	 */
	void on_destroy_entity( const Entity* entity )
	{
		remove_entity_component( entity );
	}

	virtual void add_entity_component_if_all_components_ready( const Entity* ) = 0;

}; // class System

} // namespace core::ecs
