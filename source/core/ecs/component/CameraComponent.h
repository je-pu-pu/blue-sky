#pragma once

#include <core/ecs/Component.h>

namespace core::ecs
{

/**
 * �J�����R���|�[�l���g
 *
 * �J�����̉�p�E�N���b�v���ʂ�ێ�����
 */
class CameraComponent : public Component
{
public:
	float_t fov = 90.f;
	float_t near_clip = 0.05f;
	float_t far_clip = 1500.f;

}; // class CameraComponent

} // namespace core::ecs
