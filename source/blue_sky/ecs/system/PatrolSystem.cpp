#include "PatrolSystem.h"

#include <blue_sky/ai/EnemyPerception.h>

#include <core/TimeManager.h>
#include <core/math/Quaternion.h>

#include <algorithm>
#include <cmath>

namespace blue_sky::ecs
{

namespace
{

using core::math::Vector;
using core::math::Quaternion;

constexpr float PI  = 3.14159265f;
constexpr float TAU = 6.28318531f;

/// transform の向き(yaw)を goal 方向へ turn_speed でなめらかに補間する（水平のみ）。
void face_toward( TransformComponent* transform, const Vector& goal, float turn_speed, float dt )
{
	Vector to = goal - transform->transform.get_position();
	to.set_y( 0.f );
	if ( to.length() < 1e-3f )
	{
		return;
	}

	// 前方 +Z 基準の目標 yaw（FpsPlayer と同じ規約: forward = (sin yaw, 0, cos yaw)）
	const float target_yaw = std::atan2( to.x(), to.z() );

	float yaw = 0.f, pitch = 0.f, roll = 0.f;
	transform->transform.get_rotation().get_yaw_pitch_roll( yaw, pitch, roll );

	// 最短角へ補間
	float delta = target_yaw - yaw;
	while ( delta >  PI ) { delta -= TAU; }
	while ( delta < -PI ) { delta += TAU; }

	const float max_step = turn_speed * dt;
	yaw += std::clamp( delta, -max_step, max_step );

	Quaternion q;
	q.set_yaw_pitch_roll( yaw, 0.f, 0.f );
	transform->transform.set_rotation( q );
}

} // namespace

void PatrolSystem::update( ComponentTuple& tuple ) const
{
	auto* transform  = std::get< TransformComponent* >( tuple );
	auto* patrol     = std::get< PatrolComponent* >( tuple );
	auto* alert      = std::get< AlertComponent* >( tuple );
	auto* perception = std::get< PerceptionComponent* >( tuple );

	const float dt  = time_manager_->get_delta_time();
	const Vector pos = transform->transform.get_position();

	// --- 目標地点を警戒状態で決める ---
	Vector goal;
	bool move_to_goal = false;

	switch ( alert->state )
	{
	case ai::AlertState::Alarmed:
	case ai::AlertState::Searching:
		// 最終視認地点へ移動して探す
		if ( perception->has_last_seen )
		{
			goal = perception->last_seen_position;
			move_to_goal = true;
		}
		break;

	case ai::AlertState::Suspicious:
		// その場で最終視認方向を向くだけ（移動しない）
		if ( perception->has_last_seen )
		{
			face_toward( transform, perception->last_seen_position, patrol->turn_speed, dt );
		}
		return;

	case ai::AlertState::Unaware:
	default:
		// ウェイポイント巡回
		if ( ! patrol->waypoints.empty() )
		{
			goal = patrol->waypoints[ patrol->current % patrol->waypoints.size() ];
			move_to_goal = true;
		}
		break;
	}

	if ( ! move_to_goal )
	{
		return;
	}

	// --- XZ 平面での移動 ---
	Vector to = goal - pos;
	to.set_y( 0.f );
	const float dist = to.length();

	// 巡回時のウェイポイント到達 → 待機 → 次へ
	if ( alert->state == ai::AlertState::Unaware && dist <= patrol->arrive_radius )
	{
		if ( ! patrol->waypoints.empty() )
		{
			patrol->wait_timer -= dt;
			if ( patrol->wait_timer <= 0.f )
			{
				patrol->current    = ( patrol->current + 1 ) % patrol->waypoints.size();
				patrol->wait_timer = patrol->wait_time;
			}
		}
		return;
	}

	if ( dist > 1e-3f )
	{
		const Vector dir  = to / dist;
		const float  step = patrol->move_speed * dt;

		Vector new_pos = pos + dir * std::min( step, dist );
		new_pos.set_y( pos.y() ); // 高さ維持
		transform->transform.set_position( new_pos );

		face_toward( transform, goal, patrol->turn_speed, dt );
	}
}

} // namespace blue_sky::ecs
