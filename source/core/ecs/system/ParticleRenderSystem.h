#pragma once

#include <core/ecs/System.h>

namespace core::ecs
{

/**
 * �p�[�e�B�N���`��V�X�e��
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
