#pragma once

#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/component/TransformControlComponent.h>
#include <core/ecs/System.h>

namespace core::ecs
{

/**
 * ���͂ɂ���Ďp�����X�V����V�X�e��
 *
 */
class TransformControlSystem : public System< TransformComponent, TransformControlComponent >
{
public:

public:
	TransformControlSystem() = default;
	~TransformControlSystem() = default;

	void update( ComponentTuple& ) override;

}; // class TransformControlSystem

} // namespace core::ecs
