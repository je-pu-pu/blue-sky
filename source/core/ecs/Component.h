#pragma once

#include <typeinfo>

namespace core::ecs
{

/**
 * �R���|�[�l���g���N���X
 *
 */
class Component
{
public:
	Component() = default;
	virtual ~Component() = default;

	/**
	 * ���̃R���|�[�l���g�̌^�� ID ��Ԃ�
	 */
	std::size_t get_type_id() const
	{
		return typeid( this ).hash_code();
	}

	/**
	 * �w�肵���R���|�[�l���g�̌^�� ID ��Ԃ�
	 */
	template< typename ComponentType >
	static constexpr std::size_t get_type_id()
	{
		return typeid( ComponentType ).hash_code();
	}

}; // class Component

} // namespace core::ecs
