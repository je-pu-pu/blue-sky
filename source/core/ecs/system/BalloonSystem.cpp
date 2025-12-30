#include "BalloonSystem.h"
#include <core/ecs/EntityManager.h>
#include <core/Service.h>
#include <core/physics/PhysicsManager.h>

namespace core::ecs
{

void BalloonSystem::update()
{
	auto* physics_manager = core::get_physics_manager();

	if ( ! physics_manager )
	{
		return;
	}

	// Balloon と Player の衝突を一括処理
	physics_manager->for_each_contact(
		core::physics::CollisionGroup::Balloon,
		core::physics::CollisionGroup::Player,
		[ physics_manager, this ]( void* balloon_ptr, void* player_ptr )
		{
			auto* balloon_rb = static_cast< RigidBodyComponent* >( balloon_ptr );
			auto* player_rb = static_cast< RigidBodyComponent* >( player_ptr );

			if ( ! balloon_rb || ! player_rb )
			{
				return;
			}

			// balloon_rb から BalloonComponent を探す
			for ( auto& pair : get_component_list() )
			{
				auto* bc_rigid_body = std::get< RigidBodyComponent* >( pair.second );

				if ( bc_rigid_body == balloon_rb )
				{
					auto* balloon = std::get< BalloonComponent* >( pair.second );

					// 上向きのインパルスを適用
					math::Vector impulse( 0.f, balloon->lift_force, 0.f );
					physics_manager->apply_impulse( player_rb->handle, impulse );
					break;
				}
			}
		}
	);
}

} // namespace core::ecs
