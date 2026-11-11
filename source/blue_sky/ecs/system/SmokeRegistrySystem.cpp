#include "SmokeRegistrySystem.h"

namespace blue_sky::ecs
{

void SmokeRegistrySystem::update()
{
	smokes_.clear();

	for ( auto& pair : get_component_list() )
	{
		auto* transform = std::get< TransformComponent* >( pair.second );
		auto* smoke     = std::get< SmokeComponent* >( pair.second );

		SmokeVolume v;
		v.center = transform->transform.get_position();
		v.radius = smoke->radius;
		smokes_.push_back( v );
	}
}

} // namespace blue_sky::ecs
