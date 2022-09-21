#pragma once

#include "type.h"
#include <boost/noncopyable.hpp>

/// @todo ���ꂢ�ɂ���
namespace core::graphics
{
	class GraphicsManager;
}

namespace core::ecs
{

class Entity;
class Component;

/**
 * �V�X�e���̊��N���X
 *
 */
class BaseSystem : private boost::noncopyable
{
public:
	using GraphicsManager = core::graphics::GraphicsManager;

private:
	/// �V�X�e���̎��s�D��x ( priority_ �����������Ɏ��s����� )
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
	 * ���̃V�X�e���̎��s�D��x���擾����
	 * 
	 * @return �V�X�e���̎��s�D��x ( ���l�����������Ɏ��s����� )
	 */
	int_t get_priority() const { return priority_; }

	/**
	 * ���̃V�X�e���̎��s�D��x��ݒ肷��
	 * 
	 * @param priority �V�X�e���̎��s�D��x ( ���l�����������Ɏ��s����� )
	 */
	void set_priority( int_t priority = 0 ) { priority_ = priority; }

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
