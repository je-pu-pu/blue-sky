#include "pch.h"
#include <core/math/Bullet/Transform.h>
#include <common/math.h>

/**
 * Bullet �ł� ���̌��_�Ɍ������č���肪���̉�]�ƂȂ�
 */
TEST( BulletTest, BulletTransformTest )
{
	btTransform t;
	t.setIdentity();
	t.setRotation( btQuaternion( btVector3( 0, 1, 0 ), math::degree_to_radian( 90.f ) ) );

	auto v1 = btVector3( 1, 0, 0 );
	auto v2 = t * btVector3( 0, 0, 1 ); // Z+ �̓_�� Y ���� 90 �x��]������� X+ �ɂȂ�

	EXPECT_NEAR( v1.x(), v2.x(), 1e-6f ) << v2;
	EXPECT_NEAR( v1.y(), v2.y(), 1e-6f ) << v2;
	EXPECT_NEAR( v1.z(), v2.z(), 1e-6f ) << v2;
}

TEST( BulletTest, BulletTransformTest2 )
{
	btTransform t;
	t.setIdentity();
	t.setRotation( btQuaternion( btVector3( 0, 0, 1 ), math::degree_to_radian( 90.f ) ) );

	auto v1 = btVector3( 0, -1, 0 );
	auto v2 = t * btVector3( -1, 0, 0 ); // X- �̓_�� Z ���� 90 �x��]������� Y- �ɂȂ�

	EXPECT_NEAR( v1.x(), v2.x(), 1e-6f ) << v2;
	EXPECT_NEAR( v1.y(), v2.y(), 1e-6f ) << v2;
	EXPECT_NEAR( v1.z(), v2.z(), 1e-6f ) << v2;
}