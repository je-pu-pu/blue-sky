#pragma once

#include <core/ecs/component/TransformComponent.h>
#include <blue_sky/ecs/component/TransformControlComponent.h>
#include <core/ecs/System.h>

namespace blue_sky::ecs
{

using core::ecs::System;
using core::ecs::TransformComponent;

/**
 * 入力によって姿勢を更新するシステム
 *
 */
class TransformControlSystem : public System< TransformComponent, TransformControlComponent >
{
public:

public:
	TransformControlSystem() = default;
	~TransformControlSystem() = default;

	void update( ComponentTuple& ) const override;

}; // class TransformControlSystem

} // namespace blue_sky::ecs
