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
	struct Vertex
	{
		Vector3 Position;
	};

public:
	std::array< Vertex, 1000 > particle_list;

public:
	ParticleSystemComponent();
	~ParticleSystemComponent();

}; // class ParticleSystemComponent

} // namespace core::ecs
