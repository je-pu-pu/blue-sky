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
	virtual void check_entity( const Entity* ) = 0;

public:
	BaseSystem() = default;
	virtual ~BaseSystem() = default;

	virtual void update() = 0;

	/**
	 * Entity �� Component ���ǉ����ꂽ���̏���
	 */
	template< typename ComponentType >
	void on_add_component( const Entity* entity, ComponentType* component )
	{
		// entity �ɒǉ����ꂽ component �����g�� System �̑���ΏۂɊ܂܂�Ă��邩���ׁA�܂܂�Ă��Ȃ���Ή������Ȃ�
		if ( ! has_component_type( ComponentType::get_type_id< ComponentType >() ) )
		{
			return;
		}

		// entity �� component ���ǉ����ꂽ���ɂ��Aentity �Ɏ��g�� System �̑���ΏۂƂȂ�S�Ă� Component �����������𒲂ׁA
		check_entity( entity );
	}

}; // class System

} // namespace core::ecs
