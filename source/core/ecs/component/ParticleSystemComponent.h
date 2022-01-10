#pragma once

#include <core/ecs/Component.h>
#include <core/type.h>
#include <array>

namespace core::ecs
{

/**
 * �p�[�e�B�N���V�X�e���p�R���|�[�l���g
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
