#pragma once

#include <core/ecs/System.h>
#include <core/ecs/component/TransformComponent.h>
#include <blue_sky/ecs/component/PickupComponent.h>

namespace blue_sky::ecs
{

/**
 * 一定の速さで向きを変え続ける物（SpinComponent）を回す（以前の Medal::update）
 *
 * EntityManager::update() では動かさない（本編のシーンは呼ばない）。ActiveObjectManager::update が、物体の update の後に update_spins を呼ぶ
 */
class SpinSystem : public core::ecs::System< SpinComponent, core::ecs::TransformComponent >
{
public:
	SpinSystem() = default;

	void update() override { }		///< 何もしない（上の説明のとおり）

	/// 向きを進める（毎フレーム）
	void update_spins( float_t delta_time );
};

} // namespace blue_sky::ecs
