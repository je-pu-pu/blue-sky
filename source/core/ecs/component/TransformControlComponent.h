#pragma once

#include <core/ecs/Component.h>

namespace core::ecs
{

/**
 * 入力によって姿勢を更新する事をタグ付けするためのコンポーネント
 *
 */
class TransformControlComponent : public Component
{
public:
	float yaw = 0.f;
	float pitch = 0.f;

}; // class TransformControlComponent

} // namespace core::ecs
