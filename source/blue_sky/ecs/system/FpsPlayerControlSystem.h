#pragma once

#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/component/RigidBodyComponent.h>
#include <blue_sky/ecs/component/FpsPlayerComponent.h>
#include <core/ecs/component/CameraComponent.h>
#include <core/ecs/System.h>

namespace core::input { class InputManager; }
namespace core::physics { class PhysicsManager; }
namespace core { class TimeManager; }

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
	/// 入力と物理は作り手が渡す（以前は core::Service から取っていた）
	/// time_manager : ジェットの燃料の増減に使う経過時間（nullptr なら燃料は増減しない）
	FpsPlayerControlSystem( const core::input::InputManager* input, core::physics::PhysicsManager* physics_manager, const core::TimeManager* time_manager = nullptr )
		: input_( input )
		, physics_manager_( physics_manager )
		, time_manager_( time_manager )
	{ }
	~FpsPlayerControlSystem() = default;

	void update() override;

private:
	/// 対象 1 つぶん（update() が対象ごとに呼ぶ。仮想関数ではないので、ループの中にインライン化できる）
	void update_entity( ComponentTuple& ) const;

private:
	const core::input::InputManager* input_;
	core::physics::PhysicsManager* physics_manager_;
	const core::TimeManager* time_manager_;

}; // class FpsPlayerControlSystem

} // namespace blue_sky::ecs
