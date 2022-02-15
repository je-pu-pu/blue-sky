#pragma once

#include <core/ecs/System.h>
#include <core/ecs/component/ParticleSystemComponent.h>
#include <core/type.h>

namespace core::ecs
{

/**
 * パーティクルシステム
 *
 */
class ParticleSystem : public System< ParticleSystemComponent >
{
public:
	// static constexpr uint_t ID = 1; // typeid( ParticleSystem* );

public:
	ParticleSystem() = default;
	~ParticleSystem() = default;

	void update() override;

}; // class ParticleSystem

} // namespace core::ecs
