#pragma once

#include <typeinfo>

namespace core::ecs
{

/**
 * コンポーネント基底クラス
 *
 */
class Component
{
public:
	Component() = default;
	virtual ~Component() = default;

	/**
	 * このコンポーネントの型の ID を返す
	 */
	std::size_t get_type_id() const
	{
		return typeid( this ).hash_code();
	}

	/**
	 * 指定したコンポーネントの型の ID を返す
	 */
	template< typename ComponentType >
	static constexpr std::size_t get_type_id()
	{
		return typeid( ComponentType ).hash_code();
	}

}; // class Component

} // namespace core::ecs
