#pragma once

#include <core/ecs/component/TransformComponent.h>
#include <blue_sky/ecs/component/PerceptionComponent.h>
#include <core/ecs/System.h>

namespace core::physics { class PhysicsManager; }

namespace blue_sky::ecs
{

using core::ecs::System;
using core::ecs::TransformComponent;

class TargetRegistrySystem;

/**
 * 知覚 System（S2 知覚）。
 *
 * PerceptionComponent を持つ敵ごとに、TargetRegistrySystem が集めた対象位置に対して
 * 「視界錐の内側か（幾何）」＋「遮蔽が無いか（物理レイ LOS）」を判定し、結果を
 * PerceptionComponent に書き込む。判定の幾何本体は blue_sky::ai の純関数。
 *
 * TargetRegistrySystem より後、AlertSystem より前に走らせる。
 */
class PerceptionSystem : public System< TransformComponent, PerceptionComponent >
{
public:
	/// physics_manager : 遮蔽のレイを飛ばす物理の世界（作り手が渡す。以前は core::Service から取っていた）
	explicit PerceptionSystem( core::physics::PhysicsManager* physics_manager ) : physics_manager_( physics_manager ) { }

	/// 対象の一覧（TargetRegistrySystem）を頭で一度だけ引き、敵ごとに判定する（以前は敵ごとに引いていた）
	void update() override;
	void update( ComponentTuple& ) const override;

private:
	core::physics::PhysicsManager* physics_manager_;

	/// 敵 1 体ぶんの判定（registry が nullptr なら見えないとする）
	void perceive( ComponentTuple&, const TargetRegistrySystem* registry ) const;

}; // class PerceptionSystem

} // namespace blue_sky::ecs
