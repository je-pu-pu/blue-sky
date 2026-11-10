#pragma once

#include "BaseSystem.h"
#include "Entity.h"
#include "EntityManager.h"
#include <unordered_map>

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
	using ComponentList = std::unordered_map< EntityId, ComponentTuple >;
private:
	
	template< typename Head, typename ... Tail >
	struct ComponentTypeList
	{
		/**
		 * ComponentTypes に指定した Component の型が含まれているかを調べる
		 */
		static constexpr bool has_component_type( ComponentTypeId component_type_id )
		{
			if ( Head::template get_type_id< Head >() == component_type_id )
			{
				return true;
			}

			return ComponentTypeList< Tail ... >::has_component_type( component_type_id );
		}

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
		static constexpr bool has_component_type( ComponentTypeId component_type_id )
		{
			return Head::template get_type_id< Head >() == component_type_id;
		}

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

	/**
	 * 指定した Component が自身の System の操作対象に含まれているかを調べる
	 * 
	 * @param component_type_id ComponentTypeId
	 * @return bool 指定した Component が自身の System の操作対象に含まれている場合は true を、含まれていなければ false を返す
	 */
	bool has_component_type( ComponentTypeId component_type_id ) const override
	{
		return ComponentTypeList< ComponentTypes ... >::has_component_type( component_type_id  );
	}

	/**
	 * 操作対象から entity の Component を削除する
	 * 
	 * @param entity entity
	 */
	void remove_entity_component( const Entity* entity ) override
	{
		auto i = component_list_.find( entity->get_id() );

		if ( i == component_list_.end() )
		{
			return;
		}

		on_remove_entity( i->second );
		component_list_.erase( i );
	}

protected:
	/**
	 * Entity がこの System の対象から外れる直前に呼ばれる（コンポーネントを外した・Entity を消した）
	 *
	 * コンポーネントはまだ生きている。System が作った物（剛体など）を片付けるのに使う。
	 * ここで対象の一覧（get_component_list）を変えてはいけない。
	 * System が壊れるとき（remove_system・clear）は呼ばれないので、派生のデストラクタで片付ける
	 */
	virtual void on_remove_entity( ComponentTuple& ) { }

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
		ComponentTuple component_tuple;

		if ( ComponentTypeList< ComponentTypes ... >::gather_entity_components( * get_entity_manager(), entity, component_tuple ) )
		{
			component_list_.emplace( entity->get_id(), component_tuple );
		}
	}

	virtual void update() override
	{
		for ( auto& i : get_component_list() )
		{
			update( i.second );
		}
	}

	virtual void update( ComponentTuple& ) const
	{

	}

}; // class System

} // namespace core::ecs
