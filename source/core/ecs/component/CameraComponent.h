#pragma once

#include <core/ecs/Component.h>

namespace core::ecs
{

/**
 * カメラコンポーネント
 *
 * カメラの画角・クリップ平面を保持する
 */
class CameraComponent : public Component
{
public:
	float_t fov = 90.f;
	float_t near_clip = 0.05f;
	float_t far_clip = 1500.f;

}; // class CameraComponent

} // namespace core::ecs
