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
	// const size_t ID = 1;

	struct Vertex
	{
		Vector3 Position;
	};

public:
	std::array< Vertex, 1000 > particle_list;

public:
	ParticleSystemComponent() = default;
	~ParticleSystemComponent() = default;

}; // class ParticleSystemComponent

} // namespace core::ecs
