#include "PatrolSystem.h"

#include <blue_sky/ai/EnemyPerception.h>

#include <core/TimeManager.h>
#include <core/math/Quaternion.h>
#include <core/nav/NavMesh.h>

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

void PatrolSystem::update()
{
	for_each( [ this ] ( ComponentTuple& t ) { update_entity( t ); } );
}

void PatrolSystem::update_entity( ComponentTuple& tuple ) const
{
	auto* transform  = std::get< TransformComponent* >( tuple );
	auto* patrol     = std::get< PatrolComponent* >( tuple );
	auto* alert      = std::get< AlertComponent* >( tuple );
	auto* perception = std::get< PerceptionComponent* >( tuple );

	if ( perception->downed )
	{
		return; // 無力化中は動かない
	}

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

	// --- 経路の決定（navmesh があれば経路追従、無ければ直線） ---
	Vector move_target = goal;   // 直線移動時はゴールへ直行
	bool   on_link     = false;  // オフメッシュ接続（階段/ジャンプ）を横断中か

	if ( navmesh_ && navmesh_->is_built() )
	{
		// いまリンク横断中（直前の経路点がオフメッシュ入口）か。
		// 橋/階段リンクの上は navmesh 外なので、そこで find_path すると最寄りポリゴンが
		// 見つからず失敗し、非リンク分岐で歩行面が無く縁停止して動けなくなる。
		// → 横断中はゴールが変わっても経路を張り直さず、渡り切ってから再探索する。
		bool mid_link = false;
		if ( ! patrol->path.empty() && patrol->current_path_point >= 1
		     && patrol->current_path_point < patrol->path.size()
		     && ( patrol->current_path_point - 1 ) < patrol->path_flags.size() )
		{
			mid_link = ( patrol->path_flags[ patrol->current_path_point - 1 ]
			             & core::nav::NavMesh::off_mesh_connection_flag() ) != 0;
		}

		// ゴールが大きく動いた or 経路が無ければ張り直す（リンク横断中を除く）
		Vector goal_delta = goal - patrol->path_goal;
		goal_delta.set_y( 0.f );
		if ( ! mid_link && ( patrol->path.empty() || goal_delta.length() > 1.0f ) )
		{
			navmesh_->find_path( pos, goal, patrol->path, & patrol->path_flags );
			patrol->current_path_point = 0;
			patrol->path_goal = goal;
		}

		// 到達した経路点をスキップして、次の点を目標にする
		while ( patrol->current_path_point < patrol->path.size() )
		{
			Vector d = patrol->path[ patrol->current_path_point ] - pos;
			d.set_y( 0.f );
			if ( d.length() <= 0.6f ) { ++patrol->current_path_point; }
			else { break; }
		}
		if ( patrol->current_path_point < patrol->path.size() )
		{
			move_target = patrol->path[ patrol->current_path_point ];

			// 直前の経路点がオフメッシュ入口フラグを持つなら、今の区間はリンク横断。
			const std::size_t prev = patrol->current_path_point - 1;
			if ( patrol->current_path_point >= 1 && prev < patrol->path_flags.size() )
			{
				on_link = ( patrol->path_flags[ prev ] & core::nav::NavMesh::off_mesh_connection_flag() ) != 0;
			}
		}
	}

	// --- 最終ゴールへの到達判定（巡回時のウェイポイント送り） ---
	Vector to_goal = goal - pos;
	to_goal.set_y( 0.f );
	if ( alert->state == ai::AlertState::Unaware && to_goal.length() <= patrol->arrive_radius )
	{
		if ( ! patrol->waypoints.empty() )
		{
			patrol->wait_timer -= dt;
			if ( patrol->wait_timer <= 0.f )
			{
				patrol->current    = ( patrol->current + 1 ) % patrol->waypoints.size();
				patrol->wait_timer = patrol->wait_time;
				patrol->path.clear();   // 次のウェイポイントへ経路を張り直す
			}
		}
		return;
	}

	if ( on_link )
	{
		// --- リンク横断（階段/ジャンプ）: 歩行面スナップせず move_target へ 3D 直進 ---
		// リンクの下には地面 navmesh があるため、スナップすると地面に張り付いて登れない。
		// 入口→出口を 3D で直進し、階段を上る/下りるように高さも変える。
		const Vector to3 = move_target - pos;
		const float  d3  = to3.length();
		if ( d3 > 1e-3f )
		{
			const Vector new_pos = pos + ( to3 / d3 ) * std::min( patrol->move_speed * dt, d3 );
			transform->transform.set_position( new_pos );
			face_toward( transform, move_target, patrol->turn_speed, dt );
		}
		return;
	}

	// --- move_target へ XZ 移動し、y は足元の歩行面へスナップする ---
	// キネマティック（重力なし）だが常に歩行面へ張り付くので、空中を滑降できない。
	Vector to = move_target - pos;
	to.set_y( 0.f );
	const float dist = to.length();
	if ( dist > 1e-3f )
	{
		const Vector dir  = to / dist;
		const float  step = patrol->move_speed * dt;
		Vector new_pos = pos + dir * std::min( step, dist );

		if ( navmesh_ && navmesh_->is_built() )
		{
			// 進む先に歩行面があるときだけ移動し、その高さへスナップする。
			// 面が無い＝崖/縁（例: 追跡でプレイヤーが navmesh 外にいて経路が張れず直進する場合）。
			// 踏み外して空中を滑空しないよう、縁では止まる（向きだけ合わせる）。
			float ground_y = pos.y();
			if ( navmesh_->sample_height( new_pos, ground_y ) )
			{
				new_pos.set_y( ground_y );
				transform->transform.set_position( new_pos );
			}
		}
		else
		{
			new_pos.set_y( pos.y() );   // navmesh 無し（平面）: そのまま移動
			transform->transform.set_position( new_pos );
		}

		face_toward( transform, move_target, patrol->turn_speed, dt );
	}
}

} // namespace blue_sky::ecs
