#include "ContactCacheSystem.h"

namespace blue_sky::ecs
{

void ContactCacheSystem::update_caches( float_t delta_time )
{
	for ( auto& i : get_component_list() )
	{
		std::get< ContactCacheComponent* >( i.second )->cache.update( delta_time );
	}
}

} // namespace blue_sky::ecs
