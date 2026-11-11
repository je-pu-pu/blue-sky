#include "TargetRegistrySystem.h"

namespace blue_sky::ecs
{

void TargetRegistrySystem::update()
{
	targets_.clear();

	for ( auto& pair : get_component_list() )
	{
		auto* transform   = std::get< TransformComponent* >( pair.second );
		auto* perceivable = std::get< PerceivableComponent* >( pair.second );

		PerceptibleTarget t;
		t.position       = transform->transform.get_position();
		t.emitting_noise = perceivable->emitting_noise;
		t.noise_loudness = perceivable->noise_loudness;
		targets_.push_back( t );
	}
}

} // namespace blue_sky::ecs
