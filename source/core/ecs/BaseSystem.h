#pragma once

#include "type.h"

namespace core::ecs
{

class Entity;
class Component;

/**
 * �V�X�e���̊��N���X
 *
 */
class BaseSystem
{
public:

protected:
	virtual bool has_component_type( ComponentTypeId ) const = 0;
	virtual void remove_entity_component( const Entity* ) = 0;

public:
	BaseSystem() = default;
	virtual ~BaseSystem() = default;

	virtual void update() = 0;

	/**
	 * Entity �� Component ���ǉ����ꂽ���̏���
	 * 
	 * @param entity ComponentType �� Component ���ǉ����ꂽ Entity
	 */
	template< typename ComponentType >
	void on_add_component( const Entity* entity )
	{
		// entity �ɒǉ����ꂽ component �����g�� System �̑���ΏۂɊ܂܂�Ă��邩���ׁA�܂܂�Ă��Ȃ���Ή������Ȃ�
		if ( ! has_component_type( ComponentType::get_type_id< ComponentType >() ) )
		{
			return;
		}

		// entity �� component ���ǉ����ꂽ���ɂ��Aentity �Ɏ��g�� System �̑���ΏۂƂȂ�S�Ă� Component �������Ă���΁AComponent ���V�X�e���̑���ΏۂƂ��Ēǉ�����
		add_entity_component_if_all_components_ready( entity );
	}

	/**
	 * Entity ���� Component ����菜���ꂽ���̏���
	 * 
	 * @param entity ComponentType �� Component ����菜���ꂽ Entity
	 */
	template< typename ComponentType >
	void on_remove_component( const Entity* entity )
	{
		// entity �����菜���ꂽ Component �����g�� System �̑���ΏۂɊ܂܂�Ă��邩���ׁA�܂܂�Ă��Ȃ���Ή������Ȃ�
		if ( ! has_component_type( ComponentType::get_type_id< ComponentType >() ) )
		{
			return;
		}

		// entity �����菜���ꂽ Component �����g�� System �̑���Ώۂł���΁Aentity �� Component ���V�X�e�������菜��
		remove_entity_component( entity );
	}

	virtual void add_entity_component_if_all_components_ready( const Entity* ) = 0;

}; // class System

} // namespace core::ecs
