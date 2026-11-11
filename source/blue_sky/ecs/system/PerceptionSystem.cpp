#include "PerceptionSystem.h"
#include "TargetRegistrySystem.h"
#include "SmokeRegistrySystem.h"

#include <blue_sky/ai/EnemyPerception.h>

#include <core/ecs/EntityManager.h>
#include <core/physics/PhysicsManager.h>

#include <cfloat>

namespace blue_sky::ecs
{

void PerceptionSystem::update()
{
	const auto* registry = get_entity_manager()->get_system< TargetRegistrySystem >();

	// 煙（視線を遮る球）。無ければ nullptr（＝遮蔽なし）
	const auto* smoke_registry = get_entity_manager()->get_system< SmokeRegistrySystem >();

	for ( auto& i : get_component_list() )
	{
		perceive( i.second, registry, smoke_registry );
	}
}

void PerceptionSystem::perceive( ComponentTuple& tuple, const TargetRegistrySystem* registry, const SmokeRegistrySystem* smoke_registry ) const
{
	using core::math::Vector;

	auto* transform  = std::get< TransformComponent* >( tuple );
	auto* perception = std::get< PerceptionComponent* >( tuple );

	perception->target_visible = false;

	if ( perception->downed )
	{
		return; // 無力化中は何も知覚しない
	}

	if ( ! registry )
	{
		return;
	}

	const Vector eye     = transform->transform.get_position() + Vector( 0.f, perception->eye_height, 0.f );
	const Vector forward = transform->transform.forward();
	auto* physics = physics_manager_;

	float best_dist = FLT_MAX;

	for ( const auto& target : registry->targets() )
	{
		const Vector& tpos = target.position;
		const Vector to = tpos - eye;
		const float dist = to.length();

		// 既に検知した対象より近くなければ採らないので、レイを飛ばさない
		if ( dist >= best_dist )
		{
			continue;
		}

		bool detected = false;

		// 1. 視覚: 視界錐の内側 かつ 遮蔽(LOS)なし
		if ( dist >= 1e-4f && ai::is_in_view_cone( eye, forward, perception->cone, tpos ) )
		{
			// 対象カプセルの手前まで遮蔽が無いか（対象自身に当たらないよう手前で止める）
			const Vector dir     = to / dist;
			const Vector los_end = tpos - dir * 0.6f;

			bool blocked = ( physics && physics->ray_test( eye, los_end ) );

			// 煙で視線が遮られるか（聴覚は遮らない）
			if ( ! blocked && smoke_registry )
			{
				for ( const auto& s : smoke_registry->smokes() )
				{
					if ( ai::segment_intersects_sphere( eye, los_end, s.center, s.radius ) )
					{
						blocked = true;
						break;
					}
				}
			}

			detected = ! blocked; // 見えた
		}

		// 2. 聴覚: 対象が発音中で可聴半径内（向き・遮蔽は無関係）
		if ( ! detected && target.emitting_noise )
		{
			detected = ai::hears_noise( eye, perception->hearing_radius, tpos, target.noise_loudness ); // 聞こえた
		}

		// 最も近い検知対象を採用（視覚・聴覚どちらでも）
		if ( detected )
		{
			best_dist = dist;
			perception->target_visible     = true;
			perception->last_seen_position = tpos;
			perception->has_last_seen      = true;
		}
	}
}

} // namespace blue_sky::ecs
