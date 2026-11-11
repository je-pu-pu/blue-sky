#pragma once

#include <core/ecs/component/TransformComponent.h>
#include <blue_sky/ecs/component/AlertComponent.h>
#include <blue_sky/ecs/component/PerceptionComponent.h>
#include <core/ecs/System.h>

namespace blue_sky::ecs
{

using core::ecs::System;
using core::ecs::TransformComponent;

/**
 * 警報伝播 System（systemic な連鎖）。
 *
 * 警報(Alarmed)状態の敵の周囲 radius 内にいる他の敵に警戒を伝播させ、
 * 最終視認地点を共有して合流させる。「1体が気づくと周囲も気づく」。
 *
 * enabled で ON/OFF を切り替えられる（imgui から操作）。
 * AlertSystem の後・PatrolSystem の前に走らせる。
 */
class AlarmPropagationSystem : public System< TransformComponent, AlertComponent, PerceptionComponent >
{
public:
	bool  enabled = true;   ///< 伝播の ON/OFF（切替用）
	float radius  = 14.f;   ///< 伝播半径 (m)

	void update() override;

}; // class AlarmPropagationSystem

} // namespace blue_sky::ecs
