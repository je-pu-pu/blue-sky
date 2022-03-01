#include "ParticleRenderSystem.h"
#include <iostream>

namespace core::ecs
{

void ParticleRenderSystem::update()
{
	std::cout << "ParticleRenderSystem::update()" << std::endl;

	for ( auto& i : get_component_list() )
	{
		auto* transform = std::get< TransformComponent* >( i.second );

		std::cout << "postion : " << transform->position << std::endl;
	}
}

} // namespace core::ecs
