#include "FpsPlayerControlSystem.h"
#include <core/Service.h>
#include <core/input/InputManager.h>
#include <core/physics/PhysicsManager.h>
#include <common/math.h>
#include <cmath>

namespace core::ecs
{

void FpsPlayerControlSystem::update( ComponentTuple& component_tuple ) const
{
	using Vector = core::math::Vector;
	using Button = core::input::Button;

	const auto* input = core::get_input_manager();
	auto* physics_manager = core::get_physics_manager();

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
		Vector position = transform->transform.get_position();
		// 足元から少し下に向かってレイを飛ばす
		Vector ray_from = position + Vector( 0.f, 0.1f, 0.f );
		Vector ray_to = position - Vector( 0.f, 0.2f, 0.f );

		fps_player->is_grounded = physics_manager->ray_test_excluding( ray_from, ray_to, rigid_body->handle );
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

		// --- 6. ジャンプ処理 ---
		if ( fps_player->is_grounded && input->push( Button::JUMP ) )
		{
			Vector jump_impulse( 0.f, fps_player->jump_impulse, 0.f );
			physics_manager->apply_impulse( rigid_body->handle, jump_impulse );
		}
	}

	// --- 7. カメラの Transform を更新 ---
	// カメラの回転 = yaw + pitch
	transform->transform.get_rotation().set_yaw_pitch_roll(
		fps_player->yaw,
		fps_player->pitch,
		0.f
	);
}

} // namespace core::ecs
