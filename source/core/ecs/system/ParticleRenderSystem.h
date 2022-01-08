#pragma once

#include <core/ecs/System.h>

namespace core::ecs
{

/**
 * パーティクル描画システム
 *
 */
class ParticleRenderSystem : public System
{
public:

public:
	ParticleRenderSystem();
	~ParticleRenderSystem();

	void update() override;

}; // class ParticleRenderSystem

} // namespace core::ecs
