#include "pch.h"

#include <core/physics/PhysicsManager.h>
#include <core/physics/RigidBody.h>

#include <optional>
#include <utility>

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
	a.set_entity_tag( 42 );		// 同じ剛体かをタグで見分ける

	core::physics::RigidBody b( std::move( a ) );

	EXPECT_FALSE( a.is_valid() );
	EXPECT_EQ( std::optional< core::ecs::EntityId >( 42 ), b.get_entity_tag() );
	EXPECT_EQ( 1, physics_manager.get_rigid_body_count() );

	a.reset();	// 空の持ち主を消しても何もしない
	EXPECT_EQ( 1, physics_manager.get_rigid_body_count() );

	// 持っている持ち主へムーブで上書きすると、上書きされた方の剛体は外れる
	core::physics::RigidBody c = physics_manager.create_rigid_body( core::physics::RigidBodyCreateInfo() );
	ASSERT_EQ( 2, physics_manager.get_rigid_body_count() );

	c = std::move( b );
	EXPECT_EQ( std::optional< core::ecs::EntityId >( 42 ), c.get_entity_tag() );
	EXPECT_EQ( 1, physics_manager.get_rigid_body_count() );

	physics_manager.destroy_rigid_body( c );	// destroy_rigid_body は reset と同じ
	EXPECT_FALSE( c.is_valid() );
	EXPECT_EQ( 0, physics_manager.get_rigid_body_count() );
}
