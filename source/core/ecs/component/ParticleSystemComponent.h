#pragma once

#include <core/ecs/Component.h>
#include <core/type.h>
#include <array>

namespace core::ecs
{

/**
 * パーティクルシステム用コンポーネント
 *
 */
class ParticleSystemComponent : public Component
{
public:
	struct Vertex
	{
		Vector3 position;
		Vector3 velocity;
	};

	using ParticleList = std::array< Vertex, 10000 >;

public:
	 ParticleList particle_list;

public:
	ParticleSystemComponent() = default;
	~ParticleSystemComponent() = default;

}; // class ParticleSystemComponent

} // namespace core::ecs
