#include "pch.h"
#include <core/math/DirectXMath/Vector.h>
#include <core/math/DirectXMath/Quaternion.h>
#include <core/math/DirectXMath/Matrix.h>
#include <common/math.h>

using core::math::direct_x_math::Vector;
using core::math::direct_x_math::Quaternion;
using core::math::direct_x_math::Matrix;

using namespace math::literals;

/**
 * DirectX では、回転軸の + 側から原点を見た状態で、右回り ( 時計回り ) が正の回転となる
 */
TEST( DirectXMathTest, RotationY )
{
	auto m = Matrix::identity();
	m.set_rotation_y( 90._deg );

	auto v = Vector::Forward * m; // Z+ の点を Y 軸で 90 度回転させると X+ になる

	EXPECT_NEAR( v.x(), Vector::Right.x(), 1e-6f ) << v;
	EXPECT_NEAR( v.y(), Vector::Right.y(), 1e-6f ) << v;
	EXPECT_NEAR( v.z(), Vector::Right.z(), 1e-6f ) << v;
}

TEST( DirectXMathTest, RotationX )
{
	auto m = Matrix::identity();
	m.set_rotation_x( 90._deg );

	auto v = Vector::Up * m; // Y+ の点を X 軸で 90 度回転させると Z+ になる

	EXPECT_NEAR( v.x(), Vector::Forward.x(), 1e-6f ) << v;
	EXPECT_NEAR( v.y(), Vector::Forward.y(), 1e-6f ) << v;
	EXPECT_NEAR( v.z(), Vector::Forward.z(), 1e-6f ) << v;
}

TEST( DirectXMathTest, RotationZ )
{
	auto m = Matrix::identity();
	m.set_rotation_z( 90._deg );

	auto v = Vector::Right * m; // X+ の点を Z 軸で 90 度回転させると Y+ になる

	EXPECT_NEAR( v.x(), Vector::Up.x(), 1e-6f ) << v;
	EXPECT_NEAR( v.y(), Vector::Up.y(), 1e-6f ) << v;
	EXPECT_NEAR( v.z(), Vector::Up.z(), 1e-6f ) << v;
}

TEST( DirectXMathTest, RotationYaw )
{
	Quaternion q = Quaternion::from_yaw_pitch_roll( 90._deg, 0.f, 0.f );
	auto v = Vector::transform( Vector::Forward, q ); // Z+ の点を Y 軸で 90 度回転させると X+ になる

	EXPECT_NEAR( v.x(), Vector::Right.x(), 1e-6f ) << v;
	EXPECT_NEAR( v.y(), Vector::Right.y(), 1e-6f ) << v;
	EXPECT_NEAR( v.z(), Vector::Right.z(), 1e-6f ) << v;
}

TEST( DirectXMathTest, RotationPitch )
{
	Quaternion q = Quaternion::from_yaw_pitch_roll( 0.f, 90._deg, 0.f );
	auto v = Vector::transform( Vector::Up, q ); // Y+ の点を X 軸で 90 度回転させると Z+ になる

	EXPECT_NEAR( v.x(), Vector::Forward.x(), 1e-6f ) << v;
	EXPECT_NEAR( v.y(), Vector::Forward.y(), 1e-6f ) << v;
	EXPECT_NEAR( v.z(), Vector::Forward.z(), 1e-6f ) << v;
}

TEST( DirectXMathTest, RotationRoll )
{
	Quaternion q = Quaternion::from_yaw_pitch_roll( 0.f, 0.f, 90._deg );
	auto v = Vector::transform( Vector::Right, q ); // X+ の点を Z 軸で 90 度回転させると Y+ になる

	EXPECT_NEAR( v.x(), Vector::Up.x(), 1e-6f ) << v;
	EXPECT_NEAR( v.y(), Vector::Up.y(), 1e-6f ) << v;
	EXPECT_NEAR( v.z(), Vector::Up.z(), 1e-6f ) << v;
}

TEST( DirectXMathTest, QuaternionIdentity )
{
	float yaw, pitch, roll;
	Quaternion::Identity.get_yaw_pitch_roll( yaw, pitch, roll );

	EXPECT_NEAR( yaw,   0.f, 1e-6f ) << Quaternion::Identity;
	EXPECT_NEAR( pitch, 0.f, 1e-6f ) << Quaternion::Identity;
	EXPECT_NEAR( roll,  0.f, 1e-6f ) << Quaternion::Identity;
}

TEST( DirectXMathTest, QuaternionYaw )
{
	Quaternion q = Quaternion::from_yaw_pitch_roll( 90._deg, 0.f, 0.f );

	float yaw, pitch, roll;
	q.get_yaw_pitch_roll( yaw, pitch, roll );

	EXPECT_NEAR( yaw,   90._deg, 1e-6f ) << q;
	EXPECT_NEAR( pitch, 0.f,     1e-6f ) << q;
	EXPECT_NEAR( roll,  0.f,     1e-6f ) << q;
}

TEST( DirectXMathTest, QuaternionPitch )
{
	Quaternion q = Quaternion::from_yaw_pitch_roll( 0.f, 90._deg, 0.f );

	float yaw, pitch, roll;
	q.get_yaw_pitch_roll( yaw, pitch, roll );

	EXPECT_NEAR( yaw,   0.f,    1e-6f );
	EXPECT_NEAR( pitch, 90._deg, 1e-6f );
	EXPECT_NEAR( roll,  0.f,    1e-6f );
}

TEST( DirectXMathTest, QuaternionRoll )
{
	Quaternion q = Quaternion::from_yaw_pitch_roll( 0.f, 0.f, 90._deg );

	float yaw, pitch, roll;
	q.get_yaw_pitch_roll( yaw, pitch, roll );

	EXPECT_NEAR( yaw,   0.f,    1e-6f );
	EXPECT_NEAR( pitch, 0.f,    1e-6f );
	EXPECT_NEAR( roll,  90._deg, 1e-6f );
}

TEST( DirectXMathTest, QuaternionYawPitch )
{
	Quaternion q = Quaternion::from_yaw_pitch_roll( 45_deg, 45_deg, 0.f );

	float yaw, pitch, roll;
	q.get_yaw_pitch_roll( yaw, pitch, roll );

	EXPECT_NEAR( yaw,   45_deg, 1e-6f );
	EXPECT_NEAR( pitch, 45_deg, 1e-6f );
	EXPECT_NEAR( roll,  0.f,    1e-6f );
}

TEST( DirectXMathTest, QuaternionYawRoll )
{
	Quaternion q = Quaternion::from_yaw_pitch_roll( 45_deg, 0_deg, 45_deg );

	float yaw, pitch, roll;
	q.get_yaw_pitch_roll( yaw, pitch, roll );

	EXPECT_NEAR( yaw,   45_deg, 1e-6f );
	EXPECT_NEAR( pitch,  0_deg, 1e-6f );
	EXPECT_NEAR( roll,  45_deg, 1e-6f );
}

TEST( DirectXMathTest, QuaternionPitchRoll )
{
	Quaternion q = Quaternion::from_yaw_pitch_roll( 0_deg, 45_deg, 45_deg );

	float yaw, pitch, roll;
	q.get_yaw_pitch_roll( yaw, pitch, roll );

	EXPECT_NEAR( yaw,    0_deg, 1e-6f );
	EXPECT_NEAR( pitch, 45_deg, 1e-6f );
	EXPECT_NEAR( roll,  45_deg, 1e-6f );
}

TEST( DirectXMathTest, RotationYawPitch )
{
	Quaternion q = Quaternion::from_yaw_pitch_roll( 45_deg, 45_deg, 0_deg );
	auto v = Vector::transform( Vector::Forward, q );

	EXPECT_NEAR( v.x(),  std::sin( 45_deg ) * std::sin( 45_deg ), 1e-6f ) << v;
	EXPECT_NEAR( v.y(), -std::sin( 45_deg ), 1e-6f ) << v;
	EXPECT_NEAR( v.z(),  std::cos( 45_deg ) * std::cos( 45_deg ), 1e-6f ) << v;
}

TEST( DirectXMathTest, RotationYawRoll )
{
	Quaternion q = Quaternion::from_yaw_pitch_roll( 45_deg, 0_deg, 45_deg );
	auto v = Vector::transform( Vector::Right, q );

	EXPECT_NEAR( v.x(),  std::cos( 45_deg ) * std::cos( 45_deg ), 1e-6f ) << v;
	EXPECT_NEAR( v.y(),  std::sin( 45_deg ), 1e-6f ) << v;
	EXPECT_NEAR( v.z(), -std::cos( 45_deg ) * std::cos( 45_deg ), 1e-6f ) << v;
}

TEST( DirectXMathTest, RotationPitchRoll )
{
	Quaternion q = Quaternion::from_yaw_pitch_roll( 0_deg, 45_deg, 45_deg );
	auto v = Vector::transform( Vector::Up, q );

	EXPECT_NEAR( v.x(), -std::sin( 45_deg ), 1e-6f ) << v;
	EXPECT_NEAR( v.y(),  std::cos( 45_deg ) * std::cos( 45_deg ), 1e-6f ) << v;
	EXPECT_NEAR( v.z(),  std::sin( 45_deg ) * std::cos( 45_deg ), 1e-6f ) << v;
}
