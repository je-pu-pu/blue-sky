#include "TransformControlSystem.h"
#include <blue_sky/GameMain.h> /// @todo core から blue_sky を参照しているのは変なので、直す
#include <blue_sky/Input.h> /// @todo core から blue_sky を参照しているのは変なので、直す

#include <common/math.h>
#include <iostream>

#include <DirectXTK/SimpleMath.h>

namespace core::ecs
{

void TransformControlSystem::update( ComponentTuple& component_tuple )
{
	using namespace blue_sky;

	const auto* input = GameMain::get_instance()->get_input();

	auto* transform = std::get< TransformComponent* >( component_tuple );
	const auto moving_speed = input->press( Input::Button::R2 ) ? 0.3f : 0.1f;

	if ( input->press( Input::Button::LEFT ) )
	{
		transform->transform.get_position() -= transform->transform.right() * moving_speed;
	}

	if ( input->press( Input::Button::RIGHT ) )
	{
		transform->transform.get_position() += transform->transform.right() * moving_speed;
	}

	if ( input->press( Input::Button::UP ) )
	{
		transform->transform.get_position() += transform->transform.forward() * moving_speed;
	}

	if ( input->press( Input::Button::DOWN ) )
	{
		transform->transform.get_position() -= transform->transform.forward() * moving_speed;
	}

	if ( input->press( Input::Button::L ) )
	{
		transform->transform.get_position() += transform->transform.up() * moving_speed;
	}

	if ( input->press( Input::Button::L2 ) )
	{
		transform->transform.get_position() -= transform->transform.up() * moving_speed;
	}
	
#if 0 // DirectXTK SimpleMath
	static DirectX::SimpleMath::Quaternion q;
	auto v = q.ToEuler();

	v.y += math::degree_to_radian( 0.1f );
	v.x = math::degree_to_radian( 45.f );

	q = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll( v.y, v.x, v.z );
	auto up = DirectX::SimpleMath::Vector3::Transform( DirectX::SimpleMath::Vector3::Up, q );

	std::cout <<
		math::radian_to_degree( v.x ) << ", " <<
		math::radian_to_degree( v.y ) << ", " <<
		math::radian_to_degree( v.z ) << std::endl;

	std::cout <<
		"up : " << up.x << ", " << up.y << ", " << up.z << std::endl;
#endif


#if 0 // Bullet LinserMath Transform EulerYPR
	static btTransform t;
	float_t yaw, pitch, roll;

	t.getBasis().getEulerYPR( yaw, pitch, roll );

	// yaw += math::degree_to_radian( 0.1f );
	pitch += math::degree_to_radian( 0.1f );
	// roll += math::degree_to_radian( 0.1f );

	t.getBasis().setEulerYPR( yaw, pitch, roll );

	const auto up = t.getBasis() * btVector3( 0.f, 1.f, 0.f );

	std::cout <<
		math::radian_to_degree( yaw ) << ", " <<
		math::radian_to_degree( pitch ) << ", " <<
		math::radian_to_degree( roll ) << std::endl;

	std::cout <<
		"up : " << up.x() << ", " << up.y() << ", " << up.z() << std::endl;
#endif

#if 0
	float x, y, z;

	// auto q = t.getRotation();
	// q.getEulerZYX( x, y, z );

	// q.getEulerZYX( yaw, pitch, roll );

	// t.getBasis().getEulerZYX( x, y, z );

	yaw += math::degree_to_radian( 0.1f );
	// y += math::degree_to_radian( 0.1f );

	t.getBasis().setEulerYPR( yaw, pitch, roll );
	// t.getBasis().setEulerZYX( roll, pitch, yaw );
	// t.getBasis().setEulerZYX( x, y, z );

	// q.setEulerZYX( yaw, pitch, roll );
	// t.setRotation( q );

	// q.setEulerZYX( x, y, z );
	// t.setRotation( q );

	// t.getBasis().getEulerYPR( yaw, pitch, roll );

	const auto up = t.getBasis() * btVector3( 0.f, 1.f, 0.f );

	std::cout <<
		math::radian_to_degree( x ) << ", " <<
		math::radian_to_degree( y ) << ", " <<
		math::radian_to_degree( z ) << std::endl;

	std::cout <<
		"up : " << up.x() << ", " << up.y() << ", " << up.z() << std::endl;
#endif

	/*
	t.getBasis().getEulerYPR( yaw, pitch, roll );

	yaw += math::degree_to_radian( 0.1f );

	std::cout <<
		math::radian_to_degree( yaw   ) << ", " <<
		math::radian_to_degree( pitch ) << ", " <<
		math::radian_to_degree( roll  ) << std::endl;

	t.getBasis().setEulerYPR( yaw, pitch, roll );
	*/

#if 0
	auto r = transform->transform.get_rotation();
	r.get_yaw_pitch_roll( yaw, pitch, roll );
	
	/// yaw を + すると、描画上 roll が + しているように見える
	/// pitch を + すると、描画上 yaw が + しているように見える
	/// roll を + すると、描画上 pitch が + しているように見える

	yaw += math::degree_to_radian( 0.1f );
	// pitch += math::degree_to_radian( 0.1f );
	// roll += math::degree_to_radian( 0.1f );

	r.set_yaw_pitch_roll( yaw, pitch, roll );

	transform->transform.set_rotation( r );

	std::cout <<
		math::radian_to_degree( yaw   ) << ", " <<
		math::radian_to_degree( pitch ) << ", " <<
		math::radian_to_degree( roll  ) << std::endl;
#endif

	/*
	float_t yaw = r.y() + math::degree_to_radian( 0.1f );
	float_t pitch = r.x(); // // math::degree_to_radian( 45.f );
	float_t roll = r.z();
	*/

	/*
	while ( yaw > 180.f )
	{
		yaw -= 360.f;
	}
	*/

	/*
	Quaternion q( yaw, pitch, roll );
	*/
	

	/// @todo 回転をちゃんとやる
	// const auto q = Quaternion( , , 0.f ) * transform->transform.get_rotation();
	// const auto zyx = q.get_euler_zyx();
	// q.set_euler_zyx( 0.f, zyx.y(), zyx.x() );
	// transform->transform.set_rotation( q );

	/*
	camera_->rotate_degree_target() += Vector( get_input()->get_mouse_dy() * 90.f,  * 90.f, 0.f );
	camera_->rotate_degree_target().set_x( math::clamp( camera_->rotate_degree_target().x(), -90.f, +90.f ) );
	*/

	float yaw, pitch, roll;
	transform->transform.get_rotation().get_yaw_pitch_roll( yaw, pitch, roll );

	// yaw += 0.001f; // input->get_mouse_dx();
	pitch += 0.001f; // input->get_mouse_dy();

	transform->transform.get_rotation().set_yaw_pitch_roll( yaw, pitch, 0 );

	auto v = DirectX::SimpleMath::Vector3( 0.f );
	auto f = DirectX::SimpleMath::Vector3::Forward;
	auto q = DirectX::SimpleMath::Quaternion::Identity;

	auto f2 = DirectX::SimpleMath::Vector3::Transform( f, q );
	
}

} // namespace core::ecs
