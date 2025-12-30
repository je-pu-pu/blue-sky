#pragma once

#include <core/ecs/System.h>
#include <core/ecs/component/BalloonComponent.h>
#include <core/ecs/component/RigidBodyComponent.h>

namespace core::ecs
{

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
	void update() override;
	void update( ComponentTuple& ) const override {}  // 個別処理は行わない
};

} // namespace core::ecs
