#pragma once

#include "BaseSystem.h"
#include "Entity.h"
#include "EntityManager.h"
#include "SparseIndex.h"

#include <tuple>
#include <utility>
#include <vector>

namespace core::ecs
{

/**
 * システムの基底クラス
 *
 */
template< typename ... ComponentTypes >
class System : public BaseSystem
{
public:
	using ComponentTuple = std::tuple< std::add_pointer_t< ComponentTypes > ... >;

	/**
	 * 対象の一覧（Entity の番号と部品のポインタの組を、密な配列に並べる）
	 *
	 * 以前は unordered_map< EntityId, ComponentTuple > で、回すとハッシュ表の節を飛び飛びにたどっていた。
	 * 部品のポインタは部品を外すまで動かない（StablePool）ので、組に持っておけば回すときに引き直さなくてよい。
	 * 並びは決まっていない（外すときに末尾と入れ替える）。作った順が要る System は自分で並べる
	 */
	using ComponentList = std::vector< std::pair< EntityId, ComponentTuple > >;
private:
	
	template< typename Head, typename ... Tail >
	struct ComponentTypeList
	{
		/**
		 * Entity に ComponentTypes の Component が含まれているかを調べ、含まれていればそれを component_tuple に追加する
		 * 
		 * @return bool Entity に ComponentTypes の Component が全て含まれていれば true を、ひとつでも含まれていないものがあれば false を返す
		 */
		/// コンポーネントは持ち主の EntityManager から得る（System は書き換えるので非 const。Entity は const の目印として受け取る）
		template< typename Manager >
		static bool gather_entity_components( Manager& manager, const Entity* entity, ComponentTuple& component_tuple )
		{
			auto component = manager.template get_component< Head >( entity );

			std::get< std::add_pointer_t< Head > >( component_tuple ) = component;

			if ( ! component )
			{
				return false;
			}			

			return ComponentTypeList< Tail ... >::gather_entity_components( manager, entity, component_tuple );
		}
	};

	template< typename Head >
	struct ComponentTypeList< Head >
	{
		template< typename Manager >
		static bool gather_entity_components( Manager& manager, const Entity* entity, ComponentTuple& component_tuple )
		{
			auto component = manager.template get_component< Head >( entity );

			std::get< std::add_pointer_t< Head > >( component_tuple ) = component;

			return component;
		}
	};

private:
	/// 操作対象となる Component の一覧
	ComponentList component_list_;

	/// Entity の番号 → component_list_ の添字（外すときに引く）
	SparseIndex component_index_;

	/**
	 * 指定した Component が自身の System の操作対象に含まれているかを調べる
	 * 
	 * @param component_type_id ComponentTypeId
	 * @return bool 指定した Component が自身の System の操作対象に含まれている場合は true を、含まれていなければ false を返す
	 */
	bool has_component_type( ComponentTypeId component_type_id ) const override
	{
		return ( ( get_component_type_id< ComponentTypes >() == component_type_id ) || ... );
	}

	/**
	 * 操作対象から entity の Component を削除する
	 * 
	 * @param entity entity
	 */
	void remove_entity_component( const Entity* entity ) override
	{
		const auto id = static_cast< EntityId >( entity->get_id() );

		if ( ! component_index_.contains( id ) )
		{
			return;
		}

		on_remove_entity( id, component_list_[ component_index_.find( id ) ].second );

		// 知らせた先で一覧が変わっていないことは決まり（on_remove_entity の説明）だが、添字は引き直す
		const auto index = component_index_.find( id );

		if ( index == SparseIndex::INVALID )
		{
			return;
		}

		// 末尾と入れ替えて消す（並びは変わる）
		const auto last = static_cast< SparseIndex::Index >( component_list_.size() - 1 );

		if ( index != last )
		{
			component_list_[ index ] = std::move( component_list_[ last ] );
			component_index_.set( component_list_[ index ].first, index );
		}

		component_list_.pop_back();
		component_index_.erase( id );
	}

protected:
	/**
	 * Entity がこの System の対象から外れる直前に呼ばれる（コンポーネントを外した・Entity を消した）
	 *
	 * コンポーネントはまだ生きている。System が作った物（剛体など）を片付けるのに使う。
	 * ここで対象の一覧（get_component_list）を変えてはいけない（部品を足す・外す・Entity を消すことも含む）。
	 * 同じく、対象の一覧を回している間に、この System の対象が増減する操作をしてはいけない（配列の反復子が無効になる）。
	 * System が壊れるとき（remove_system・clear）は呼ばれないので、派生のデストラクタで片付ける
	 *
	 * @param id 外れる Entity の番号（自分で番号の順の一覧を持つ System が、その一覧から外すのに使う）
	 */
	virtual void on_remove_entity( EntityId /* id */, ComponentTuple& ) { }

	ComponentList& get_component_list() { return component_list_; }
	const ComponentList& get_component_list() const { return component_list_; }

public:
	/// この System が処理対象としている Entity 数（テスト・デバッグ用）
	std::size_t debug_entity_count() const { return component_list_.size(); }

public:
	System() = default;
	virtual ~System() = default;

	/**
	 * entity をチェックし、entity が自身の System の操作対象となる Component を全て持っていれば、操作対象として追加する
	 * 
	 * @param entity チェックする entity
	 */
	void add_entity_component_if_all_components_ready( const Entity* entity ) override
	{
		const auto id = static_cast< EntityId >( entity->get_id() );

		if ( component_index_.contains( id ) )
		{
			return;
		}

		ComponentTuple component_tuple;

		if ( ComponentTypeList< ComponentTypes ... >::gather_entity_components( * get_entity_manager(), entity, component_tuple ) )
		{
			component_index_.set( id, static_cast< SparseIndex::Index >( component_list_.size() ) );
			component_list_.emplace_back( id, component_tuple );
		}
	}

	// update() は派生が必ず書く（BaseSystem の純粋仮想）。対象は for_each か get_component_list() で回す。
	// 以前は System::update() が対象ごとに仮想関数 update( ComponentTuple& ) を呼んでいて、
	// 1 体ごとに仮想関数の呼び出しの手間がかかり、中身が小さい System ではインライン化もできなかった

	/**
	 * 対象を 1 つずつ f( ComponentTuple& ) に渡す（派生の update() の中で使う）
	 *
	 * f は型が分かっているので、ループの中にインライン化できる。回している間に、この System の対象を増減してはいけない
	 */
	template< typename F >
	void for_each( F&& f )
	{
		for ( auto& i : component_list_ )
		{
			f( i.second );
		}
	}

	template< typename F >
	void for_each( F&& f ) const
	{
		for ( const auto& i : component_list_ )
		{
			f( i.second );
		}
	}

}; // class System

} // namespace core::ecs
