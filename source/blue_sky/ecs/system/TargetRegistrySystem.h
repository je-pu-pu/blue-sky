#pragma once

#include <core/ecs/component/TransformComponent.h>
#include <blue_sky/ecs/component/PerceivableComponent.h>
#include <core/ecs/System.h>
#include <core/math/Vector.h>

#include <vector>

namespace blue_sky::ecs
{

using core::ecs::System;
using core::ecs::TransformComponent;

/// 知覚され得る対象1体分の情報（位置＋今フレームの発音状態）。
struct PerceptibleTarget
{
	core::math::Vector position;
	bool  emitting_noise = false;
	float noise_loudness = 0.f;
};

/**
 * 知覚対象レジストリ System。
 *
 * PerceivableComponent を持つ実体の位置を毎フレーム収集し、知覚 System(PerceptionSystem)へ供給する。
 * 敵AIが「プレイヤー」の具体型に触れずに、対象の位置だけを得るための中立サービス
 * （システム階層スタックの「敵とプレイヤーは世界状態越しにのみ相互作用」を実装で担保）。
 *
 * 知覚 System より先に走らせること（priority を小さく）。
 */
class TargetRegistrySystem : public System< TransformComponent, PerceivableComponent >
{
public:
	/// 収集した対象（位置＋発音）。知覚 System が読む。
	const std::vector< PerceptibleTarget >& targets() const { return targets_; }

	void update() override;

private:
	std::vector< PerceptibleTarget > targets_;

}; // class TargetRegistrySystem

} // namespace blue_sky::ecs
