#include "pch.h"

#include <blue_sky/ecs/system/PhysicsSyncSystem.h>
#include <blue_sky/ecs/component/PhysicsBodyComponent.h>

#include <core/ecs/EntityManager.h>
#include <core/ecs/component/TransformComponent.h>
#include <core/physics/PhysicsManager.h>
#include <core/math/Quaternion.h>

#include <common/math.h>

/**
 * 本編の剛体と位置の正（TransformComponent）の同期（ecs::PhysicsSyncSystem）と、
 * 剛体の持ち主を部品が持つこと（ecs::PhysicsBodyComponent・PhysicsManager が返す持ち主）のテスト
 */

using blue_sky::ecs::PhysicsSyncSystem;
using blue_sky::ecs::PhysicsBodyComponent;
using core::ecs::EntityManager;
using core::ecs::TransformComponent;

namespace
{

core::Transform make_transform( float_t x, float_t y, float_t z, float_t yaw_degree = 0.f )
{
	core::Transform t;
	t.set_identity();
	t.set_position( core::Vector( x, y, z ) );
	t.set_rotation( core::Quaternion::from_yaw_pitch_roll( math::degree_to_radian( yaw_degree ), 0.f, 0.f ) );
	return t;
}

core::Transform identity()
{
	return make_transform( 0.f, 0.f, 0.f );
}

/// 本編の物体と同じ作り方（形を共有・Bullet の既定の摩擦と振り分け）の箱
core::physics::RigidBody make_box( core::physics::PhysicsManager& physics, float_t mass )
{
	core::physics::RigidBodyCreateInfo info;
	info.shape_size = core::Vector( 0.5f, 0.5f, 0.5f );
	info.mass = mass;
	info.transform = identity();
	info.offset = identity();
	info.friction = core::physics::RigidBodyCreateInfo::BulletDefaultFriction;
	info.share_shape = true;
	info.use_default_collision_filter = true;

	return physics.create_rigid_body( info );
}

/// 剛体の部品を持つ Entity を作る（持ち主を持たせる）
core::ecs::Entity* create_body_entity( EntityManager& manager, core::physics::RigidBody&& owner, float_t height_offset )
{
	auto* entity = manager.create_entity();
	entity->add_component< TransformComponent >()->transform = identity();

	auto* body = entity->add_component< PhysicsBodyComponent >();
	body->owner = std::move( owner );
	body->height_offset = height_offset;

	return entity;
}

} // namespace

TEST( PhysicsSyncSystemTest, 剛体の中心は足元から高さのずれだけ上で_送って読むと元に戻る )
{
	// 以前の GameObject::commit_transform・update_transform と同じ計算
	core::physics::PhysicsManager physics;
	auto owner = make_box( physics, 1.f );

	const core::Transform foot = make_transform( 1.f, 2.f, 3.f, 90.f );
	PhysicsSyncSystem::write_body( owner, 0.75f, foot );

	const core::Vector center = owner.get_world_transform( core::Vector::Zero ).get_position();
	EXPECT_NEAR( 1.f, center.x(), 1e-5f );
	EXPECT_NEAR( 2.75f, center.y(), 1e-5f );
	EXPECT_NEAR( 3.f, center.z(), 1e-5f );
	EXPECT_NEAR( 2.75f, owner.get_center_of_mass_position().y(), 1e-5f );

	const core::Transform read = PhysicsSyncSystem::read_body( owner, 0.75f );
	EXPECT_NEAR( 1.f, read.get_position().x(), 1e-5f );
	EXPECT_NEAR( 2.f, read.get_position().y(), 1e-5f );
	EXPECT_NEAR( 3.f, read.get_position().z(), 1e-5f );
	EXPECT_NEAR( foot.forward().x(), read.forward().x(), 1e-5f );		// 回転も戻る（前が +X を向いたまま）
	EXPECT_NEAR( foot.forward().z(), read.forward().z(), 1e-5f );
}

TEST( PhysicsSyncSystemTest, 物理を進めた後に剛体の位置を位置の正へ写す )
{
	core::physics::PhysicsManager physics;
	EntityManager manager;
	manager.add_system< PhysicsSyncSystem >( 0 );

	auto* entity = create_body_entity( manager, make_box( physics, 1.f ), 0.5f );
	auto* body = entity->get_component< PhysicsBodyComponent >()->get();

	PhysicsSyncSystem::write_body( * body, 0.5f, make_transform( 0.f, 10.f, 0.f ) );
	physics.step_simulation( 1.f / 60.f );	// 落ちる

	manager.get_system< PhysicsSyncSystem >()->update_transforms();

	const float_t y = entity->get_component< TransformComponent >()->transform.get_position().y();
	EXPECT_LT( y, 10.f );
	EXPECT_GT( y, 9.f );
	EXPECT_NEAR( body->get_center_of_mass_position().y() - 0.5f, y, 1e-5f );
}

TEST( PhysicsSyncSystemTest, Entity_を消すと持ち主の剛体は世界から外れ_剛体の無い部品は写さない )
{
	// 以前は物体を壊しても剛体が世界に残り、シーンの終わり（clear）にまとめて消していた。
	// 以前は複製（clone）が剛体を共有するだけの部品を持てたが、複製できる種類が無くなったので部品は持ち主だけを持つ
	core::physics::PhysicsManager physics;
	EntityManager manager;
	manager.add_system< PhysicsSyncSystem >( 0 );

	auto* original = create_body_entity( manager, make_box( physics, 1.f ), 0.5f );
	ASSERT_EQ( 1, physics.get_rigid_body_count() );

	// 剛体の無い部品（ステージを読む前のプレイヤー）は写さない
	auto* empty = manager.create_entity();
	empty->add_component< TransformComponent >()->transform = make_transform( 0.f, 5.f, 0.f );
	empty->add_component< PhysicsBodyComponent >();
	EXPECT_EQ( nullptr, empty->get_component< PhysicsBodyComponent >()->get() );

	manager.get_system< PhysicsSyncSystem >()->update_transforms();
	EXPECT_FLOAT_EQ( 5.f, empty->get_component< TransformComponent >()->transform.get_position().y() );

	manager.destroy_entity( empty->get_id() );
	EXPECT_EQ( 1, physics.get_rigid_body_count() );

	manager.destroy_entity( original->get_id() );
	EXPECT_EQ( 0, physics.get_rigid_body_count() );
}

TEST( PhysicsSyncSystemTest, 地面は物理の世界が持ち_clear_は持ち主のいる剛体を消さず外すだけ )
{
	core::physics::PhysicsManager physics;

	physics.add_ground( core::Vector( 10.f, 1.f, 10.f ) );
	auto owner = make_box( physics, 1.f );
	ASSERT_EQ( 2, physics.get_rigid_body_count() );

	owner.reset();
	EXPECT_EQ( 1, physics.get_rigid_body_count() );		// 地面は残る

	auto other = make_box( physics, 1.f );
	physics.clear();

	EXPECT_EQ( 0, physics.get_rigid_body_count() );		// 地面は消え、持ち主のいる剛体は世界から外れる
	EXPECT_TRUE( other.is_valid() );						// 持ち主の剛体は消えていない（後で持ち主が消す。二重に消さない）
}
