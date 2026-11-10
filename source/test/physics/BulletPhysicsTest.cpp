#include "pch.h"

#include <core/physics/Bullet/BulletPhysics.h>

// ---- 物理（BulletPhysics）: 形の共有と、世界を出さない問い合わせ ----

namespace
{

core::math::Transform identity()
{
	core::math::Transform t;
	t.set_identity();
	return t;
}

} // namespace

TEST( BulletPhysicsTest, 同じ形と大きさの剛体は形を共有し大きさが違えば別にする )
{
	BulletPhysics physics;

	btRigidBody* a = physics.add_box_rigid_body( identity(), identity(), core::math::Vector( 1.f, 2.f, 3.f ), 1.f );
	btRigidBody* b = physics.add_box_rigid_body( identity(), identity(), core::math::Vector( 1.f, 2.f, 3.f ), 1.f );
	btRigidBody* c = physics.add_box_rigid_body( identity(), identity(), core::math::Vector( 1.f, 2.f, 4.f ), 1.f );
	btRigidBody* d = physics.add_cylinder_rigid_body( identity(), identity(), core::math::Vector( 1.f, 2.f, 3.f ), 1.f );

	// 以前は剛体ごとに形を作っていた（同じ大きさでもメモリが重複していた）
	EXPECT_EQ( a->getCollisionShape(), b->getCollisionShape() );
	EXPECT_NE( a->getCollisionShape(), c->getCollisionShape() );
	EXPECT_NE( a->getCollisionShape(), d->getCollisionShape() );		// 種類が違えば別
}

TEST( BulletPhysicsTest, 線分の問い合わせで剛体に当たる )
{
	BulletPhysics physics;
	physics.add_box_rigid_body( identity(), identity(), core::math::Vector( 1.f, 1.f, 1.f ), 0.f );

	// 世界そのもの（btDynamicsWorld）を出さず、問い合わせだけで調べられる
	btCollisionWorld::ClosestRayResultCallback hit( btVector3( 0.f, 10.f, 0.f ), btVector3( 0.f, -10.f, 0.f ) );
	physics.ray_test( btVector3( 0.f, 10.f, 0.f ), btVector3( 0.f, -10.f, 0.f ), hit );
	EXPECT_TRUE( hit.hasHit() );

	btCollisionWorld::ClosestRayResultCallback miss( btVector3( 5.f, 10.f, 0.f ), btVector3( 5.f, -10.f, 0.f ) );
	physics.ray_test( btVector3( 5.f, 10.f, 0.f ), btVector3( 5.f, -10.f, 0.f ), miss );
	EXPECT_FALSE( miss.hasHit() );
}
