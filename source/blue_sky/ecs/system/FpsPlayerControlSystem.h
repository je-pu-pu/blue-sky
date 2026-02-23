#pragma once

#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/component/RigidBodyComponent.h>
#include <blue_sky/ecs/component/FpsPlayerComponent.h>
#include <core/ecs/component/CameraComponent.h>
#include <core/ecs/System.h>

namespace blue_sky::ecs
{

using core::ecs::System;
using core::ecs::RigidBodyComponent;
using core::ecs::TransformComponent;
using core::ecs::CameraComponent;

/**
 * FPS プレイヤー制御システム
 *
 * FpsPlayerComponent, RigidBodyComponent, TransformComponent, CameraComponent を
 * 持つ Entity に対して、入力に基づく移動・視点制御を行う。
 */
class FpsPlayerControlSystem : public System<
	FpsPlayerComponent,
	RigidBodyComponent,
	TransformComponent,
	CameraComponent >
{
public:
	FpsPlayerControlSystem() = default;
	~FpsPlayerControlSystem() = default;

	void update( ComponentTuple& ) const override;

}; // class FpsPlayerControlSystem

} // namespace blue_sky::ecs
