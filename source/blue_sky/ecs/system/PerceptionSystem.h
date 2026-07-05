#pragma once

#include <core/ecs/component/TransformComponent.h>
#include <blue_sky/ecs/component/PerceptionComponent.h>
#include <core/ecs/System.h>

namespace blue_sky::ecs
{

using core::ecs::System;
using core::ecs::TransformComponent;

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
	void update( ComponentTuple& ) const override;

}; // class PerceptionSystem

} // namespace blue_sky::ecs
