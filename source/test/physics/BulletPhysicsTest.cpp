#include "pch.h"

#include <core/physics/Bullet/BulletPhysics.h>
#include <core/physics/PhysicsManager.h>
#include <core/physics/RigidBody.h>

#include <utility>

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

	auto a = physics.add_box_rigid_body( identity(), identity(), core::math::Vector( 1.f, 2.f, 3.f ), 1.f );
	auto b = physics.add_box_rigid_body( identity(), identity(), core::math::Vector( 1.f, 2.f, 3.f ), 1.f );
	auto c = physics.add_box_rigid_body( identity(), identity(), core::math::Vector( 1.f, 2.f, 4.f ), 1.f );
	auto d = physics.add_cylinder_rigid_body( identity(), identity(), core::math::Vector( 1.f, 2.f, 3.f ), 1.f );

	// 以前は剛体ごとに形を作っていた（同じ大きさでもメモリが重複していた）
	EXPECT_EQ( a->getCollisionShape(), b->getCollisionShape() );
	EXPECT_NE( a->getCollisionShape(), c->getCollisionShape() );
	EXPECT_NE( a->getCollisionShape(), d->getCollisionShape() );		// 種類が違えば別
}

TEST( BulletPhysicsTest, 線分の問い合わせで剛体に当たる )
{
	BulletPhysics physics;
	auto box = physics.add_box_rigid_body( identity(), identity(), core::math::Vector( 1.f, 1.f, 1.f ), 0.f );	// 持ち主を捨てると世界から外れる

	// 世界そのもの（btDynamicsWorld）を出さず、問い合わせだけで調べられる
	btCollisionWorld::ClosestRayResultCallback hit( btVector3( 0.f, 10.f, 0.f ), btVector3( 0.f, -10.f, 0.f ) );
	physics.ray_test( btVector3( 0.f, 10.f, 0.f ), btVector3( 0.f, -10.f, 0.f ), hit );
	EXPECT_TRUE( hit.hasHit() );

	btCollisionWorld::ClosestRayResultCallback miss( btVector3( 5.f, 10.f, 0.f ), btVector3( 5.f, -10.f, 0.f ) );
	physics.ray_test( btVector3( 5.f, 10.f, 0.f ), btVector3( 5.f, -10.f, 0.f ), miss );
	EXPECT_FALSE( miss.hasHit() );
}

// ---- 剛体の持ち主（core::physics::RigidBody。ムーブだけできる RAII） ----

TEST( RigidBodyTest, 持ち主が壊れると剛体は世界から外れる )
{
	core::physics::PhysicsManager physics_manager;

	{
		core::physics::RigidBody body = physics_manager.create_rigid_body( core::physics::RigidBodyCreateInfo() );

		ASSERT_TRUE( body.is_valid() );
		EXPECT_EQ( 1, physics_manager.get_rigid_body_count() );
	}

	// 以前は destroy_rigid_body を呼ばないと世界に残った
	EXPECT_EQ( 0, physics_manager.get_rigid_body_count() );
}

TEST( RigidBodyTest, 作った剛体を捨てるとすぐに外れる )
{
	core::physics::PhysicsManager physics_manager;

	physics_manager.create_rigid_body( core::physics::RigidBodyCreateInfo() );

	EXPECT_EQ( 0, physics_manager.get_rigid_body_count() );
}

TEST( RigidBodyTest, ムーブすると持ち主が移り_剛体は世界に残る )
{
	core::physics::PhysicsManager physics_manager;

	core::physics::RigidBody a = physics_manager.create_rigid_body( core::physics::RigidBodyCreateInfo() );
	btRigidBody* raw = a.get();

	core::physics::RigidBody b( std::move( a ) );

	EXPECT_FALSE( a.is_valid() );
	EXPECT_EQ( raw, b.get() );
	EXPECT_EQ( 1, physics_manager.get_rigid_body_count() );

	a.reset();	// 空の持ち主を消しても何もしない
	EXPECT_EQ( 1, physics_manager.get_rigid_body_count() );

	// 持っている持ち主へムーブで上書きすると、上書きされた方の剛体は外れる
	core::physics::RigidBody c = physics_manager.create_rigid_body( core::physics::RigidBodyCreateInfo() );
	ASSERT_EQ( 2, physics_manager.get_rigid_body_count() );

	c = std::move( b );
	EXPECT_EQ( raw, c.get() );
	EXPECT_EQ( 1, physics_manager.get_rigid_body_count() );

	physics_manager.destroy_rigid_body( c );	// destroy_rigid_body は reset と同じ
	EXPECT_FALSE( c.is_valid() );
	EXPECT_EQ( 0, physics_manager.get_rigid_body_count() );
}
