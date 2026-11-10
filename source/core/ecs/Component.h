#pragma once

namespace core::ecs
{

/**
 * コンポーネント基底クラス
 *
 * 型の番号は core::ecs::get_component_type_id< T >()（type.h）で得る。
 * 以前あった get_type_id() const は typeid( this )（Component* の型）で、どの部品でも同じ値を返していた。呼ぶ所も無かったので消した
 */
class Component
{
public:
	Component() = default;
	virtual ~Component() = default;

}; // class Component

} // namespace core::ecs
