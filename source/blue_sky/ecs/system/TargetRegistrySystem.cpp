#include "TargetRegistrySystem.h"

namespace blue_sky::ecs
{

void TargetRegistrySystem::update()
{
	targets_.clear();

	for ( auto& pair : get_component_list() )
	{
		auto* transform = std::get< TransformComponent* >( pair.second );
		targets_.push_back( transform->transform.get_position() );
	}
}

} // namespace blue_sky::ecs
