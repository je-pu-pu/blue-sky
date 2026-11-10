#pragma once

#include "Entity.h"
#include "Component.h"
#include "BaseSystem.h"
#include "StablePool.h"
#include "type.h"


#include <vector>
#include <unordered_map>

#include <memory>
#include <tuple>
#include <algorithm>

namespace core::ecs
{

/**
 * Entity・Component・System を持つ
 *
 * 組み立ての根（GameMain）が 1 つ持ち、シーンは Scene::get_entity_manager() で受け取る。テストは自分で作る。
 * 以前はシングルトンで、テストどうしが同じものを共有していた
 */
class EntityManager
{
private:
	/// 部品の型ごとの置き場（型を消して持つための基底）
	class BaseComponentStorage
	{
	public:
		virtual ~BaseComponentStorage() = default;

		/// id の部品を壊す（無ければ何もしない）
		virtual void erase( EntityId id ) = 0;
	};

	/// 型 T の部品の置き場。部品は値で持ち、1 つずつ new しない。場所は外すまで動かない（StablePool）
	template< typename ComponentType >
	class ComponentStorage : public BaseComponentStorage
	{
	public:
		StablePool< ComponentType > pool;

		void erase( EntityId id ) override { pool.erase( id ); }
	};

	// Entity の一覧（Entity* は消すまで動かない）
	// 以前は unordered_map< EntityId, Entity > で、作る・消す・find のたびにハッシュ表を引いていた
	StablePool< Entity > entity_list_;

	// 部品の置き場（添字は部品の型の番号 get_component_type_id。番号は 0 から詰めて振るので添字で引く。まだ使っていない型は nullptr）
	// 以前は型ごとの unordered_map< EntityId, unique_ptr< Component > > で、部品を 1 つずつ new していた
	std::vector< std::unique_ptr< BaseComponentStorage > > component_list_;

	/// 型 T の部品の置き場（まだ無ければ nullptr）
	template< typename ComponentType >
	ComponentStorage< ComponentType >* find_storage() const
	{
		const auto component_type_id = get_component_type_id< ComponentType >();

		if ( component_type_id >= component_list_.size() )
		{
			return nullptr;
		}

		return static_cast< ComponentStorage< ComponentType >* >( component_list_[ component_type_id ].get() );
	}

	// ( SystemTypeId => System の実体 ) のマップ ( System の実体を保持する )
	std::unordered_map< SystemTypeId, std::unique_ptr< BaseSystem > > system_map_;

	// System の一覧 ( 常に実行順にソートされている事が保証されている )
	std::vector< BaseSystem* > system_list_;

	/// 次に付与する Entity ID
	EntityId next_entity_id_ = 0;

public:
	EntityManager() = default;
	~EntityManager() { clear(); }	///< clear と同じ順で壊す（System が先）。System が使う物（PhysicsManager）はこれより長く生きること
	EntityManager( const EntityManager& ) = delete;
	EntityManager& operator = ( const EntityManager& ) = delete;

	/**
	 * Entity を生成する
	 */
	Entity* create_entity()
	{
		Entity* e = entity_list_.emplace( next_entity_id_, next_entity_id_, this );

		next_entity_id_++;

		return e;
	}

	/**
	 * 指定した Entity を破棄する
	 *
	 * 先に全 System の対象から外し（コンポーネントはまだ生きている）、次に全コンポーネントを外し、最後に Entity を消す。
	 * 以前は Entity だけを消していて、コンポーネントが残り、System は消えた Entity のコンポーネントを指したままだった。
	 * 無い id（消した後・作っていない）なら何もしない
	 */
	void destroy_entity( EntityId id )
	{
		Entity* entity = entity_list_.find( id );

		if ( ! entity )
		{
			return;
		}

		for ( auto& s : system_list_ )
		{
			s->on_destroy_entity( entity );
		}

		for ( auto& storage : component_list_ )
		{
			if ( storage )
			{
				storage->erase( id );
			}
		}

		entity_list_.erase( id );
	}

	/**
	 * 指定した id の Entity が生きているか
	 *
	 * EntityId は二度と使わない番号（clear でも戻さない）なので、消した Entity の id で別の Entity を指すことは無い（世代は要らない）
	 */
	bool is_alive( EntityId id ) const
	{
		return entity_list_.contains( id );
	}

	/**
	 * 指定した id の Entity を探す
	 *
	 * @return Entity（無い・消した後なら nullptr）。ポインタはその Entity を消すまで有効なので、持ち続けるなら id で持つ
	 */
	Entity* find( EntityId id )
	{
		return entity_list_.find( id );
	}

	const Entity* find( EntityId id ) const
	{
		return entity_list_.find( id );
	}

	/// 生きている Entity の数（テスト・デバッグ用）
	std::size_t debug_entity_count() const { return entity_list_.size(); }

	/**
	 * Entity に Component を追加する
	 *
	 * 部品は型ごとの置き場に値で置く。返すポインタは、その部品を外す（Entity を消す）まで動かない
	 *
	 * @param e Entity
	 * @return 追加した Component（既に持っていれば何もせず nullptr）
	 */
	template< typename ComponentType >
	ComponentType* add_component( const Entity* e )
	{
		static_assert( std::is_base_of_v< Component, ComponentType >, "部品は core::ecs::Component を継承すること" );

		const auto component_type_id = get_component_type_id< ComponentType >();

		if ( component_type_id >= component_list_.size() )
		{
			component_list_.resize( component_type_id + 1 );
		}

		auto& storage = component_list_[ component_type_id ];

		if ( ! storage )
		{
			storage = std::make_unique< ComponentStorage< ComponentType > >();
		}

		auto* c = static_cast< ComponentStorage< ComponentType >* >( storage.get() )->pool.emplace( static_cast< EntityId >( e->get_id() ) );

		if ( ! c )
		{
			return nullptr;
		}

		for ( auto& s : system_list_ )
		{
			s->on_add_component< ComponentType >( e );
		}

		return c;
	}

	/**
	 * Entity の Component を得る
	 *
	 * 読むだけで何も作らない（以前は operator[] で、未登録の型の空の一覧を作っていた）
	 *
	 * @return Component（持っていなければ nullptr）
	 */
	template< typename ComponentType >
	ComponentType* get_component( const Entity* e )
	{
		auto* storage = find_storage< ComponentType >();

		return storage ? storage->pool.find( static_cast< EntityId >( e->get_id() ) ) : nullptr;
	}

	/**
	 * Entity から Component を取り除く
	 *
	 * 先に System に知らせ（コンポーネントはまだ生きている。PhysicsSystem が剛体を解放する）、その後でコンポーネントを壊す。
	 * 以前は壊してから知らせていた
	 *
	 * @param e Entity
	 */
	template< typename ComponentType >
	void remove_component( const Entity* e )
	{
		auto* storage = find_storage< ComponentType >();
		const auto id = static_cast< EntityId >( e->get_id() );

		if ( ! storage || ! storage->pool.contains( id ) )
		{
			return;
		}

		for ( auto& s : system_list_ )
		{
			s->on_remove_component< ComponentType >( e );
		}

		// System に知らせている間に同じ部品が外され得るので、もう一度 id で引いて消す（無ければ何もしない）
		storage->pool.erase( id );
	}

	/**
	 * EntityManager にシステムを追加する。
	 *
	 * 既にシステムが追加されている場合は、何もしない
	 */
	template< typename SystemType, typename ... Args >
	void add_system( int priority = 0, Args&& ... args )
	{
		const auto id = get_system_type_id< SystemType >();

		if ( system_map_.find( id ) != system_map_.end() )
		{
			return;
		}

		auto system = new SystemType( std::forward< Args >( args ) ... );
		system->set_priority( priority );
		system->set_entity_manager( this );

		system_map_.emplace( id, system );

		system_list_.push_back( system );
		std::sort( system_list_.begin(), system_list_.end(), [] ( const BaseSystem* a, const BaseSystem* b ) { return a->get_priority() < b->get_priority(); } );

		// 既存の全 Entity に対して走査を行い、 Entity が System の操作対象となる Component を全て持っていた場合、System に Component への参照を追加する
		entity_list_.for_each( [ system ] ( EntityId, Entity& entity )
		{
			system->add_entity_component_if_all_components_ready( & entity );
		} );
	}

	/**
	 * EntityManager から System を取り除く
	 *
	 * EntityManager に System が存在しない場合は、何もしない
	 */
	template< typename SystemType >
	void remove_system()
	{
		const auto id = get_system_type_id< SystemType >();

		auto i = system_map_.find( id );

		if ( i == system_map_.end() )
		{
			return;
		}

		// 2 引数の erase（remove の返す位置から末尾まで）。以前は 1 引数で、一覧に無ければ end() を消す未定義動作だった
		system_list_.erase( std::remove( system_list_.begin(), system_list_.end(), i->second.get() ), system_list_.end() );

		system_map_.erase( i );
	}

	/**
	 * EntityManager から System を取得する
	 *
	 * @return System へのポインタ (存在しない場合は nullptr)
	 */
	template< typename SystemType >
	SystemType* get_system()
	{
		const auto id = get_system_type_id< SystemType >();

		auto i = system_map_.find( id );

		if ( i == system_map_.end() )
		{
			return nullptr;
		}

		return static_cast< SystemType* >( i->second.get() );
	}

	/**
	 * EntityManager に追加されている全ての System を実行する
	 *
	 */
	/// 描く System 以外を、優先度の順に動かす（シーンの update() から呼ぶ）
	void update()
	{
		for ( auto& s : system_list_ )
		{
			if ( ! s->is_render_system() )
			{
				s->update();
			}
		}
	}

	/// 描く System（is_render_system）を、優先度の順に動かす（シーンの render() から呼ぶ。画面のクリアの後）
	void render()
	{
		for ( auto& s : system_list_ )
		{
			if ( s->is_render_system() )
			{
				s->update();
			}
		}
	}

	/**
	 * 全ての Entity, Component, System を削除する
	 *
	 * System を先に壊す（PhysicsSystem のデストラクタが、まだ生きている RigidBodyComponent の剛体を解放する）。
	 * 以前はコンポーネントを先に壊し、剛体は解放されず物理の世界にシーンをまたいで溜まっていた。
	 * EntityId は戻さない（is_alive で古い id が生き返らないように）
	 */
	void clear()
	{
		system_list_.clear();
		system_map_.clear();
		component_list_.clear();
		entity_list_.clear();
	}

}; // EntityManager

// ---- Entity の中身（自分を作った EntityManager に頼む） ----

template< typename ComponentType >
ComponentType* Entity::add_component()
{
	return manager_->add_component< ComponentType >( this );
}

template< typename ComponentType >
ComponentType* Entity::get_component()
{
	return manager_->get_component< ComponentType >( this );
}

template< typename ComponentType >
const ComponentType* Entity::get_component() const
{
	return manager_->get_component< ComponentType >( this );
}

template< typename ComponentType >
void Entity::remove_component()
{
	return manager_->remove_component< ComponentType >( this );
}

} // core::ecs
