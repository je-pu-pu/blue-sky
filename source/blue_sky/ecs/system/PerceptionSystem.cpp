#include "PerceptionSystem.h"
#include "TargetRegistrySystem.h"

#include <blue_sky/ai/EnemyPerception.h>

#include <core/ecs/EntityManager.h>
#include <core/Service.h>
#include <core/physics/PhysicsManager.h>

#include <cfloat>

namespace blue_sky::ecs
{

void PerceptionSystem::update( ComponentTuple& tuple ) const
{
	using core::math::Vector;

	auto* transform  = std::get< TransformComponent* >( tuple );
	auto* perception = std::get< PerceptionComponent* >( tuple );

	perception->target_visible = false;

	auto* registry = core::ecs::EntityManager::get_instance()->get_system< TargetRegistrySystem >();
	if ( ! registry )
	{
		return;
	}

	const Vector eye     = transform->transform.get_position() + Vector( 0.f, perception->eye_height, 0.f );
	const Vector forward = transform->transform.forward();
	auto* physics = core::get_physics_manager();

	float best_dist = FLT_MAX;

	for ( const auto& target : registry->targets() )
	{
		// 1. 幾何: 視界錐の内側か
		if ( ! ai::is_in_view_cone( eye, forward, perception->cone, target ) )
		{
			continue;
		}

		Vector to = target - eye;
		const float dist = to.length();
		if ( dist < 1e-4f )
		{
			continue;
		}

		// 2. 遮蔽(LOS): 対象カプセルの手前まで遮蔽が無いか（対象自身に当たらないよう手前で止める）
		const Vector dir     = to / dist;
		const Vector los_end = target - dir * 0.6f;
		if ( physics && physics->ray_test( eye, los_end ) )
		{
			continue; // 遮蔽あり ＝ 見えない
		}

		// 最も近い可視対象を採用
		if ( dist < best_dist )
		{
			best_dist = dist;
			perception->target_visible    = true;
			perception->last_seen_position = target;
			perception->has_last_seen      = true;
		}
	}
}

} // namespace blue_sky::ecs
