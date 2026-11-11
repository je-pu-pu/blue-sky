#include "GirlSystem.h"

#include <blue_sky/ecs/EntityObject.h>
#include <blue_sky/ecs/component/FlickerComponent.h>

#include <core/ecs/EntityManager.h>

namespace blue_sky::ecs
{

void GirlSystem::update_girls( float_t scene_elapsed_time )
{
	for ( auto& i : get_component_list() )
	{
		const GirlComponent* girl = std::get< GirlComponent* >( i.second );

		if ( girl->mode != GirlComponent::Mode::FLOAT )
		{
			continue;
		}

		core::ecs::Entity* entity = get_entity_manager()->find( i.first );
		const auto* flicker = entity->get_component< FlickerComponent >();

		EntityObject object( entity );

		// 以前の Girl::update（浮いているとき）
		object.set_velocity( core::Vector( 0.f, 0.f, 0.f ) );
		object.update_velocity_by_flicker( girl->float_base_location, flicker ? flicker->scale : 1.f, scene_elapsed_time );
	}
}

bool GirlSystem::action( core::ecs::EntityId id, const string_t& action )
{
	core::ecs::Entity* entity = get_entity_manager()->find( id );
	auto* girl = entity ? entity->get_component< GirlComponent >() : nullptr;

	if ( ! girl )
	{
		return false;
	}

	if ( action == "float" )
	{
		girl->mode = GirlComponent::Mode::FLOAT;
		return true;
	}

	return false;
}

} // namespace blue_sky::ecs
