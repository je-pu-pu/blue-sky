#include "pch.h"
#include <core/math/Bullet/Transform.h>
#include <common/math.h>

/**
 * Bullet では 軸の原点に向かって左回りが正の回転となる
 */
TEST( BulletTest, BulletTransformTest )
{
	btTransform t;
	t.setIdentity();
	t.setRotation( btQuaternion( btVector3( 0, 1, 0 ), math::degree_to_radian( 90.f ) ) );

	auto v1 = btVector3( 1, 0, 0 );
	auto v2 = t * btVector3( 0, 0, 1 ); // Z+ の点を Y 軸で 90 度回転させると X+ になる

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
	auto v2 = t * btVector3( -1, 0, 0 ); // X- の点を Z 軸で 90 度回転させると Y- になる

	EXPECT_NEAR( v1.x(), v2.x(), 1e-6f ) << v2;
	EXPECT_NEAR( v1.y(), v2.y(), 1e-6f ) << v2;
	EXPECT_NEAR( v1.z(), v2.z(), 1e-6f ) << v2;
}