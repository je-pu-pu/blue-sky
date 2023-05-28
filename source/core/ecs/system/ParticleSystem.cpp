#include "ParticleSystem.h"

#include <iostream>

namespace core::ecs
{


void ParticleSystem::update()
{
	// std::cout << get_priority() << "ParticleSystem::update()" << std::endl;

	for ( auto i : get_component_list() )
	{
		for ( auto& p : std::get< ParticleSystemComponent* >( i.second )->particle_list )
		{
			p.position.y() += 1.f;
		}
	}
}

} // namespace core::ecs
