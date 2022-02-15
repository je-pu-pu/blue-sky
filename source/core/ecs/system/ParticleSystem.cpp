#include "ParticleSystem.h"

#include <iostream>

namespace core::ecs
{


void ParticleSystem::update()
{
	for ( auto cs : get_component_list() )
	{
		for ( auto p : std::get< ParticleSystemComponent* >( cs )->particle_list )
		{
			p.position.y() += 1.f;
		}
	}

	std::cout << "ParticleSystem::update()" << std::endl;
}

} // namespace core::ecs
