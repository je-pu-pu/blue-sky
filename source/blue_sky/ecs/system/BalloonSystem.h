#pragma once

#include <core/ecs/System.h>
#include <blue_sky/ecs/component/BalloonComponent.h>
#include <core/ecs/component/RigidBodyComponent.h>

namespace core::physics { class PhysicsManager; }

namespace blue_sky::ecs
{

using core::ecs::System;
using core::ecs::RigidBodyComponent;

/**
 * 風船システム
 *
 * プレイヤーが風船に触れたら浮力を与える
 * for_each_contact を使用して一括処理
 */
class BalloonSystem : public System<
	BalloonComponent,
	RigidBodyComponent >
{
public:
	/// physics_manager : 接触を調べる物理の世界（作り手が渡す。以前は core::Service から取っていた）
	explicit BalloonSystem( core::physics::PhysicsManager* physics_manager ) : physics_manager_( physics_manager ) { }

	void update() override;

private:
	core::physics::PhysicsManager* physics_manager_;
};

} // namespace blue_sky::ecs
