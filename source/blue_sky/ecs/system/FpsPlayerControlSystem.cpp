#include "FpsPlayerControlSystem.h"
#include <core/input/InputManager.h>
#include <core/physics/PhysicsManager.h>
#include <core/TimeManager.h>
#include <common/math.h>
#include <algorithm>
#include <cmath>

namespace blue_sky::ecs
{

void FpsPlayerControlSystem::update()
{
	for_each( [ this ] ( ComponentTuple& t ) { update_entity( t ); } );
}

void FpsPlayerControlSystem::update_entity( ComponentTuple& component_tuple ) const
{
	using Vector = core::math::Vector;
	using Button = core::input::Button;

	const auto* input = input_;
	auto* physics_manager = physics_manager_;

	auto* fps_player = std::get< FpsPlayerComponent* >( component_tuple );
	auto* rigid_body = std::get< RigidBodyComponent* >( component_tuple );
	auto* transform = std::get< TransformComponent* >( component_tuple );

	// --- 1. 視点回転 (マウス入力) ---
	fps_player->yaw += input->get_mouse_dx();
	fps_player->pitch += input->get_mouse_dy();

	// yaw の正規化 (-180度, 180度]
	if ( fps_player->yaw < ::math::degree_to_radian( -180.f ) )
	{
		fps_player->yaw += ::math::degree_to_radian( 360.f );
	}
	if ( fps_player->yaw > ::math::degree_to_radian( 180.f ) )
	{
		fps_player->yaw -= ::math::degree_to_radian( 360.f );
	}

	// pitch の制限 (-89度, 89度)
	fps_player->pitch = ::math::clamp(
		fps_player->pitch,
		::math::degree_to_radian( -89.f ),
		::math::degree_to_radian( 89.f )
	);

	// --- 2. 移動入力の取得 ---
	float move_x = 0.f; // 左右
	float move_z = 0.f; // 前後

	if ( input->press( Button::LEFT ) )  { move_x -= 1.f; }
	if ( input->press( Button::RIGHT ) ) { move_x += 1.f; }
	if ( input->press( Button::UP ) )    { move_z += 1.f; }
	if ( input->press( Button::DOWN ) )  { move_z -= 1.f; }

	// --- 3. 移動速度の計算 ---
	float speed = fps_player->move_speed;
	if ( input->press( Button::R2 ) )
	{
		speed *= fps_player->sprint_multiplier;
	}

	// yaw に基づく前方・右方向ベクトル
	float cos_yaw = std::cos( fps_player->yaw );
	float sin_yaw = std::sin( fps_player->yaw );

	// 前方向 (Z+)
	Vector forward( sin_yaw, 0.f, cos_yaw );
	// 右方向 (X+)
	Vector right( cos_yaw, 0.f, -sin_yaw );

	// ワールド空間での移動ベクトル
	Vector move_dir = forward * move_z + right * move_x;

	if ( move_dir.length() > 0.001f )
	{
		move_dir.normalize();
		move_dir *= speed;
	}

	// --- 4. 接地判定 (レイキャスト、自分自身を除外) ---
	if ( rigid_body->handle.is_valid() && physics_manager )
	{
		const bool was_grounded = fps_player->is_grounded;

		Vector position = transform->transform.get_position();
		// 足元から少し下に向かってレイを飛ばす
		Vector ray_from = position + Vector( 0.f, 0.1f, 0.f );
		Vector ray_to = position - Vector( 0.f, 0.2f, 0.f );

		fps_player->is_grounded = physics_manager->ray_test_excluding( ray_from, ray_to, rigid_body->handle );

		// 着地の瞬間だけ、直前の落下速度を衝撃として出す（ダメージ化はゲーム側の判断）。
		// このフレームの Y 速度ではなく前フレームの fall_speed を使う: 物理が先に走って
		// 着地の衝突を解決済みで、ここで速度を読んでもすでに 0 になっているため。
		fps_player->landing_impact =
			( ! was_grounded && fps_player->is_grounded ) ? fps_player->fall_speed : 0.f;
	}

	// --- 5. RigidBody に速度を設定 ---
	if ( rigid_body->handle.is_valid() && physics_manager )
	{
		// 現在の Y 速度 (重力) を保持
		Vector current_velocity = physics_manager->get_rigid_body_linear_velocity( rigid_body->handle );

		Vector new_velocity(
			move_dir.x(),
			current_velocity.y(), // Y 軸は物理エンジンに任せる
			move_dir.z()
		);

		physics_manager->set_rigid_body_linear_velocity( rigid_body->handle, new_velocity );
		physics_manager->activate_rigid_body( rigid_body->handle );

		float vy = new_velocity.y();

		// --- 6. ジャンプ処理 ---
		if ( fps_player->is_grounded && input->push( Button::JUMP ) )
		{
			Vector jump_impulse( 0.f, fps_player->jump_impulse, 0.f );
			physics_manager->apply_impulse( rigid_body->handle, jump_impulse );
		}

		// --- 6.5 ジェット（JUMP 長押し＝上昇/滞空。燃料制） ---
		// 接地・空中どちらからでも離陸できる（!is_grounded 条件だと屋上等で小ジャンプでは
		// 空中に出られず離陸できなかった。長押しで上向き速度を保つ動詞に統一）。
		if ( fps_player->fly_enabled )
		{
			const float dt = time_manager_ ? time_manager_->get_delta_time() : 0.f;

			if ( input->press( Button::JUMP ) && fps_player->fly_fuel > 0.f )
			{
				// 上向き速度を一定に保つ（長押しで上昇・タップで滞空、離すと落下）
				Vector v = physics_manager->get_rigid_body_linear_velocity( rigid_body->handle );
				v.set_y( fps_player->fly_speed );
				physics_manager->set_rigid_body_linear_velocity( rigid_body->handle, v );
				vy = v.y();

				fps_player->fly_fuel = ( std::max )( 0.f, fps_player->fly_fuel - fps_player->fly_burn_rate * dt );
			}
			else if ( fps_player->is_grounded )
			{
				// 上昇していない接地時に燃料回復（屋上・階段・橋の上でも回復する）
				fps_player->fly_fuel = ( std::min )( fps_player->fly_fuel_max, fps_player->fly_fuel + fps_player->fly_regen_rate * dt );
			}
		}

		// --- 6.6 傘（降りる動詞）: 開いている間は落下速度を終端速度まで抑える。
		// 上昇には干渉せず「落ちる速さ」だけを制限する ＝ 高所から安全に降りられる。
		if ( fps_player->umbrella_open && vy < -fps_player->umbrella_fall_speed )
		{
			vy = -fps_player->umbrella_fall_speed;

			Vector v = physics_manager->get_rigid_body_linear_velocity( rigid_body->handle );
			v.set_y( vy );
			physics_manager->set_rigid_body_linear_velocity( rigid_body->handle, v );
		}

		// --- 6.7 落下速度の記録（次フレームの着地衝撃に使う） ---
		fps_player->fall_speed = ( std::max )( 0.f, -vy );
	}

	// --- 7. カメラの Transform を更新 ---
	// カメラの回転 = yaw + pitch
	transform->transform.get_rotation().set_yaw_pitch_roll(
		fps_player->yaw,
		fps_player->pitch,
		0.f
	);
}

} // namespace blue_sky::ecs
