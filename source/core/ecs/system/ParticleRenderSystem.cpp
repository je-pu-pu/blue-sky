#include "ParticleRenderSystem.h"
#include <iostream>

namespace core::ecs
{

void ParticleRenderSystem::update()
{
	std::cout << "ParticleRenderSystem::update()" << std::endl;

	for ( auto& component_tuple : get_component_list() )
	{
		auto* transform = std::get< TransformComponent* >( component_tuple );

		std::cout << "postion : " << transform->position.x() << std::endl;
	}
}

} // namespace core::ecs
