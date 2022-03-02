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
			if ( Head::get_type_id< Head >() == component_type_id )
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
		static constexpr bool gather_entity_components( const Entity* entity, ComponentTuple& component_tuple )
		{
			auto component = entity->get_component< Head >();

			std::get< std::add_pointer_t< Head > >( component_tuple ) = component;

			if ( ! component )
			{
				return false;
			}			

			return ComponentTypeList< Tail ... >::gather_entity_components( entity, component_tuple );
		}
	};

	template< typename Head >
	struct ComponentTypeList< Head >
	{
		static constexpr bool has_component_type( ComponentTypeId component_type_id )
		{
			return Head::get_type_id< Head >() == component_type_id;
		}

		static constexpr bool gather_entity_components( const Entity* entity, ComponentTuple& component_tuple )
		{
			auto component = entity->get_component< Head >();

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
	constexpr bool has_component_type( ComponentTypeId component_type_id ) const override
	{
		return ComponentTypeList< ComponentTypes ... >::has_component_type( component_type_id  );
	}

	/**
	 * entity をチェックし、entity が自身の System の操作対象となる Component を全て持っていれば、操作対象として追加する
	 * 
	 * @param entity チェックする entity
	 */
	void add_entity_component_if_all_components_ready( const Entity* entity ) override
	{
		ComponentTuple component_tuple;

		if ( ComponentTypeList< ComponentTypes ... >::gather_entity_components( entity, component_tuple ) )
		{
			component_list_.emplace( entity->get_id(), component_tuple );
		}
	}

	/**
	 * 操作対象から entity の Component を削除する
	 * 
	 * @param entity entity
	 */
	void remove_entity_component( const Entity* entity ) override
	{
		component_list_.erase( entity->get_id() );
	}

protected:
	ComponentList& get_component_list() { return component_list_; }
	const ComponentList& get_component_list() const { return component_list_; }

public:
	System() = default;
	virtual ~System() = default;

}; // class System

} // namespace core::ecs
