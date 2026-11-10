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

private:
	/// 対象 1 つぶん（update() が対象ごとに呼ぶ。仮想関数ではないので、ループの中にインライン化できる）
	void update_entity( ComponentTuple& ) const;

}; // class ParticleSystem

} // namespace core::ecs
