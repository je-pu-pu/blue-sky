#pragma once

#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/component/ParticleSystemComponent.h>
#include <core/ecs/System.h>

namespace core::ecs
{

/**
 * �p�[�e�B�N���`��V�X�e��
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
