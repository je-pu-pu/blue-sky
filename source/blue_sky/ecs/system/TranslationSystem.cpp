#include "TranslationSystem.h"

#include <blue_sky/ecs/EntityObject.h>
#include <blue_sky/ecs/component/SimpleObjectComponent.h>

#include <core/ecs/EntityManager.h>

#include <core/physics/RigidBody.h>

#include <cmath>
#include <utility>
#include <vector>

namespace blue_sky::ecs
{

void TranslationSystem::update_translations( float_t delta_time )
{
	// 剛体へ送るときに部品を引くので、番号を集めてから回す（回している間に対象は増減しないが、念のため一覧を写す）
	std::vector< std::pair< core::ecs::EntityId, TranslationComponent* > > targets;
	targets.reserve( get_component_list().size() );

	for ( auto& i : get_component_list() )
	{
		targets.emplace_back( i.first, std::get< TranslationComponent* >( i.second ) );
	}

	for ( auto& [ id, translation ] : targets )
	{
		EntityObject object( get_entity_manager()->find( id ) );
		const auto* state = object.get_entity()->get_component< SimpleObjectComponent >();

		if ( auto* body = object.get_rigid_body() )
		{
			body->set_activation_state_active();
		}

		translation->angle += delta_time * translation->speed;

		const core::Vector base = state ? state->start_location : core::Vector::Zero;

		object.set_location( base + core::Vector( std::cos( translation->angle ) * translation->width, std::sin( translation->angle ) * translation->height, std::cos( translation->angle ) * translation->depth ) );
	}
}

} // namespace blue_sky::ecs
