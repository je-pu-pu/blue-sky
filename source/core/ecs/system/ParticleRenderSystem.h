#pragma once

#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/component/ParticleSystemComponent.h>
#include <core/ecs/System.h>

namespace core::ecs
{

/**
 * パーティクル描画システム
 *
 */
class ParticleRenderSystem : public System< TransformComponent, ParticleSystemComponent >
{
public:

public:
	ParticleRenderSystem() = default;
	~ParticleRenderSystem() = default;

	void update() override;

}; // class ParticleRenderSystem

} // namespace core::ecs
