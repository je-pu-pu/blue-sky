#include "pch.h"
#include <core/math/Vector.h>
#include <core/math/DirectXMath/Matrix.h>
#include <common/math.h>

/**
 * DirectX �ł́A���̌��_�Ɍ������ĉE��肪���̉�]�ƂȂ�
 */
TEST( DirectXMathTest, DirectXMathTransformTest )
{
	using Matrix = direct_x_math::Matrix;

	auto m = Matrix::identity();
	m.set_rotation_y( math::degree_to_radian( 90.f ) );

	auto v1 = core::Vector( 1, 0, 0 );
	auto v2 = core::Vector( 0, 0, 1 ) * m; // Z+ �̓_�� Y ���� 90 �x��]������� X+ �ɂȂ�

	EXPECT_NEAR( v1.x(), v2.x(), 1e-6f ) << v2;
	EXPECT_NEAR( v1.y(), v2.y(), 1e-6f ) << v2;
	EXPECT_NEAR( v1.z(), v2.z(), 1e-6f ) << v2;
}

TEST( DirectXMathTest, DirectXMathTransformTest2 )
{
	using Matrix = direct_x_math::Matrix;

	auto m = Matrix::identity();
	m.set_rotation_z( math::degree_to_radian( 90.f ) );

	auto v1 = core::Vector( 0, 1, 0 );
	auto v2 = core::Vector( 1, 0, 0 ) * m; // X+ �̓_�� Z ���� 90 �x��]������� Y+ �ɂȂ�

	EXPECT_NEAR( v1.x(), v2.x(), 1e-6f ) << v2;
	EXPECT_NEAR( v1.y(), v2.y(), 1e-6f ) << v2;
	EXPECT_NEAR( v1.z(), v2.z(), 1e-6f ) << v2;
}