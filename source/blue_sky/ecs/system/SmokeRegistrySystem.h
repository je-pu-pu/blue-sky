#pragma once

#include <core/ecs/component/TransformComponent.h>
#include <blue_sky/ecs/component/SmokeComponent.h>
#include <core/ecs/System.h>
#include <core/math/Vector.h>

#include <vector>

namespace blue_sky::ecs
{

using core::ecs::System;
using core::ecs::TransformComponent;

/// 視線を遮る煙ボリューム1個分（球）。
struct SmokeVolume
{
	core::math::Vector center;
	float radius = 0.f;
};

/**
 * 煙レジストリ System。
 *
 * SmokeComponent を持つ実体の位置＋半径を毎フレーム収集し、知覚 System へ供給する。
 * PerceptionSystem より先に走らせる（priority を小さく）。
 */
class SmokeRegistrySystem : public System< TransformComponent, SmokeComponent >
{
public:
	const std::vector< SmokeVolume >& smokes() const { return smokes_; }

	void update() override;

private:
	std::vector< SmokeVolume > smokes_;

}; // class SmokeRegistrySystem

} // namespace blue_sky::ecs
