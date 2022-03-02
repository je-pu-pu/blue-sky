#pragma once

#include "BaseSystem.h"
#include "Entity.h"
#include "EntityManager.h"
#include <unordered_map>

namespace core::ecs
{

/**
 * �V�X�e���̊��N���X
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
		 * ComponentTypes �Ɏw�肵�� Component �̌^���܂܂�Ă��邩�𒲂ׂ�
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
		 * Entity �� ComponentTypes �� Component ���܂܂�Ă��邩�𒲂ׁA�܂܂�Ă���΂���� component_tuple �ɒǉ�����
		 * 
		 * @return bool Entity �� ComponentTypes �� Component ���S�Ċ܂܂�Ă���� true ���A�ЂƂł��܂܂�Ă��Ȃ����̂������ false ��Ԃ�
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
	/// ����ΏۂƂȂ� Component �̈ꗗ
	ComponentList component_list_;

	/**
	 * �w�肵�� Component �����g�� System �̑���ΏۂɊ܂܂�Ă��邩�𒲂ׂ�
	 * 
	 * @param component_type_id ComponentTypeId
	 * @return bool �w�肵�� Component �����g�� System �̑���ΏۂɊ܂܂�Ă���ꍇ�� true ���A�܂܂�Ă��Ȃ���� false ��Ԃ�
	 */
	constexpr bool has_component_type( ComponentTypeId component_type_id ) const override
	{
		return ComponentTypeList< ComponentTypes ... >::has_component_type( component_type_id  );
	}

	/**
	 * entity ���`�F�b�N���Aentity �����g�� System �̑���ΏۂƂȂ� Component ��S�Ď����Ă���΁A����ΏۂƂ��Ēǉ�����
	 * 
	 * @param entity �`�F�b�N���� entity
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
	 * ����Ώۂ��� entity �� Component ���폜����
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
