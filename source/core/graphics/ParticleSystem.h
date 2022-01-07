#pragma once

#include <core/type.h>
#include <array>

namespace core::graphics
{

/**
 * パーティクルシステム
 *
 */
class ParticleSystem : public 
{
public:
	struct Vertex
	{
		Vector3 Position;
	};

protected:
	std::array< Vertex, 1000 > particle_list_;

public:
	ParticleSystem();
	~ParticleSystem();

	void update();

}; // class ParticleSystem

} // namespace core::graphics
