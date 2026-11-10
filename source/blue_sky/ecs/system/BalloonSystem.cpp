#include "BalloonSystem.h"
#include <core/ecs/EntityManager.h>
#include <core/physics/PhysicsManager.h>

namespace blue_sky::ecs
{

void BalloonSystem::update()
{
	auto* physics_manager = physics_manager_;

	if ( ! physics_manager )
	{
		return;
	}

	auto* entity_manager = get_entity_manager();

	// Balloon と Player の衝突を一括処理（剛体の Entity の番号のタグから、部品を直接引く）
	// 以前は userPointer の RigidBodyComponent* を受け取り、風船の部品を対象の一覧から 1 つずつ探していた（当たり 1 つにつき O(n)）
	physics_manager->for_each_contact(
		core::physics::CollisionGroup::Balloon,
		core::physics::CollisionGroup::Player,
		[ physics_manager, entity_manager ]( core::ecs::EntityId balloon_id, core::ecs::EntityId player_id )
		{
			core::ecs::Entity* balloon_entity = entity_manager->find( balloon_id );
			core::ecs::Entity* player_entity = entity_manager->find( player_id );

			if ( ! balloon_entity || ! player_entity )
			{
				return;	// どちらかが消えた
			}

			// この System の対象（風船の部品と剛体を持つ）だけ
			auto* balloon = balloon_entity->get_component< BalloonComponent >();
			auto* player_rb = player_entity->get_component< RigidBodyComponent >();

			if ( ! balloon || ! balloon_entity->get_component< RigidBodyComponent >() || ! player_rb )
			{
				return;
			}

			// 上向きのインパルスを適用
			core::math::Vector impulse( 0.f, balloon->lift_force, 0.f );
			physics_manager->apply_impulse( player_rb->handle, impulse );
		}
	);
}

} // namespace blue_sky::ecs
