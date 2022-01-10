#pragma once

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

}; // class Component

} // namespace core::ecs
