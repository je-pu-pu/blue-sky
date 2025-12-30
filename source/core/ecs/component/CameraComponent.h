#pragma once

#include <core/ecs/Component.h>
#include <core/math/Vector.h>

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
	using Vector = core::math::Vector;

	float_t fov = 90.f;
	float_t near_clip = 0.05f;
	float_t far_clip = 1500.f;

	/// カメラ位置のオフセット (Transform の位置からの相対位置)
	Vector eye_offset = Vector( 0.f, 0.f, 0.f );

}; // class CameraComponent

} // namespace core::ecs
