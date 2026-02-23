#pragma once

#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/component/ParticleSystemComponent.h>
#include <core/ecs/System.h>
#include <core/graphics/Shader.h>

namespace core::ecs
{

/**
 * パーティクル描画システム
 *
 */
class ParticleRenderSystem : public System< TransformComponent, ParticleSystemComponent >
{
public:

private:
	std::unique_ptr< core::graphics::Shader > shader_;

public:
	explicit ParticleRenderSystem( std::unique_ptr< core::graphics::Shader > shader );
	virtual ~ParticleRenderSystem() = default;

	virtual void update() override;

}; // class ParticleRenderSystem

} // namespace core::ecs
