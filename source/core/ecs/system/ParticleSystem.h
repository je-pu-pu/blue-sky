#pragma once

#include <core/ecs/System.h>
#include <core/type.h>

namespace core::ecs
{

/**
 * �p�[�e�B�N���V�X�e��
 *
 */
class ParticleSystem : public System
{
public:
	static constexpr uint_t ID = 1; // typeid( ParticleSystem* );

public:
	ParticleSystem();
	~ParticleSystem();

	void update() override;

}; // class ParticleSystem

} // namespace core::ecs
