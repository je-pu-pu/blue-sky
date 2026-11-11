#include "SpinSystem.h"

#include <blue_sky/ecs/EntityObject.h>

#include <core/ecs/EntityManager.h>

namespace blue_sky::ecs
{

void SpinSystem::update_spins( float_t delta_time )
{
	for ( auto& i : get_component_list() )
	{
		const SpinComponent* spin = std::get< SpinComponent* >( i.second );

		EntityObject object( get_entity_manager()->find( i.first ) );

		// 以前の Medal::update : set_direction_degree( get_direction_degree() + per_sec( 90.f ) )
		// 以前の Stone::update : set_direction_degree( get_direction_degree() + 1.f )
		object.set_direction_degree( object.get_direction_degree() + spin->degree_per_second * delta_time + spin->degree_per_frame );
	}
}

} // namespace blue_sky::ecs
