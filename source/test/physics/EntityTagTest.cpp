#include "pch.h"

#include <core/physics/EntityTag.h>
#include <core/physics/PhysicsManager.h>
#include <core/ecs/EntityManager.h>
#include <core/ecs/system/PhysicsSystem.h>
#include <core/ecs/component/RigidBodyComponent.h>
#include <core/ecs/component/TransformComponent.h>

#include <blue_sky/ecs/system/BalloonSystem.h>
#include <blue_sky/ecs/component/BalloonComponent.h>

#include <btBulletDynamicsCommon.h>

#include <vector>
#include <utility>

/**
 * 剛体の userPointer に入れる Entity の番号のタグ（core/physics/EntityTag.h）と、
 * それを読む ECS 側（PhysicsSystem・PhysicsManager::for_each_contact・BalloonSystem）のテスト
 */

TEST( EntityTagTest, 番号とタグを行き来できる_0_番も区別する )
{
	for ( core::ecs::EntityId id : { 0u, 1u, 41u, 0xFFFFFFFEu } )
	{
		void* tag = core::physics::make_entity_tag( id );

		EXPECT_NE( nullptr, tag ) << id;	// 0 番も nullptr（タグ無し）にならない

		const auto read = core::physics::read_entity_tag( tag );
		ASSERT_TRUE( read.has_value() ) << id;
		EXPECT_EQ( id, * read );
	}

	EXPECT_FALSE( core::physics::read_entity_tag( nullptr ).has_value() );		// 地面など
}

TEST( EntityTagTest, 剛体にタグを付け_外せる )
{
	btCollisionObject object;

	EXPECT_FALSE( core::physics::get_entity_tag( & object ).has_value() );
	EXPECT_FALSE( core::physics::get_entity_tag( nullptr ).has_value() );

	core::physics::set_entity_tag( & object, 7 );
	ASSERT_TRUE( core::physics::get_entity_tag( & object ).has_value() );
	EXPECT_EQ( 7u, * core::physics::get_entity_tag( & object ) );

	core::physics::clear_entity_tag( & object );
	EXPECT_FALSE( core::physics::get_entity_tag( & object ).has_value() );
}

namespace {

/// 剛体の部品を持つ Entity を作る（PhysicsSystem の update で剛体ができる。大きさ 1 の箱で、y だけずらして重ねる）
core::ecs::Entity* create_body( core::ecs::EntityManager& em, short group, float y = 0.f )
{
	auto* e = em.create_entity();
	auto& transform = e->add_component< core::ecs::TransformComponent >()->transform;
	transform.set_identity();		// Transform の既定の回転は 0（単位ではない）ので、当たりが取れない
	transform.set_position( core::math::Vector( 0.f, y, 0.f ) );
	e->add_component< core::ecs::RigidBodyComponent >()->collision_group = group;
	return e;
}

} // namespace

TEST( EntityTagTest, PhysicsSystem_は剛体に_Entity_の番号のタグを付ける )
{
	// 以前は userPointer に RigidBodyComponent* を入れていた
	core::physics::PhysicsManager physics_manager;
	core::ecs::EntityManager em;
	em.add_system< core::ecs::PhysicsSystem >( 0, & physics_manager );

	em.create_entity();		// 0 番を使っておく（タグが 0 番と取り違えないことも確かめる）
	auto* e = create_body( em, core::physics::CollisionGroup::Default );
	em.update();

	const auto& handle = e->get_component< core::ecs::RigidBodyComponent >()->handle;
	ASSERT_TRUE( handle.is_valid() );

	const auto tag = handle.get_entity_tag();
	ASSERT_TRUE( tag.has_value() );
	EXPECT_EQ( e->get_id(), * tag );
}

TEST( EntityTagTest, 当たった組を_Entity_の番号で渡し_タグの無い剛体は渡さない )
{
	using core::physics::CollisionGroup::Balloon;
	using core::physics::CollisionGroup::Player;

	core::physics::PhysicsManager physics_manager;
	core::ecs::EntityManager em;
	em.add_system< core::ecs::PhysicsSystem >( 0, & physics_manager );

	auto* balloon = create_body( em, Balloon );
	auto* player = create_body( em, Player, 0.6f );

	// タグの無い剛体（地面などと同じ）。風船の組に入れても渡されない
	core::physics::RigidBodyCreateInfo info;
	info.collision_group = Balloon;
	core::physics::RigidBodyHandle untagged = physics_manager.create_rigid_body( info );

	em.update();	// 剛体を作る
	em.update();	// 重なった剛体の当たりを調べる

	std::vector< std::pair< core::ecs::EntityId, core::ecs::EntityId > > pairs;
	physics_manager.for_each_contact( Balloon, Player, [ & ] ( core::ecs::EntityId a, core::ecs::EntityId b ) { pairs.emplace_back( a, b ); } );

	ASSERT_EQ( 1u, pairs.size() );
	EXPECT_EQ( balloon->get_id(), pairs[ 0 ].first );		// group_a（風船）側が先
	EXPECT_EQ( player->get_id(), pairs[ 0 ].second );
}

TEST( EntityTagTest, BalloonSystem_は当たったプレイヤーを番号で引いて持ち上げる )
{
	core::physics::PhysicsManager physics_manager;
	core::ecs::EntityManager em;
	em.add_system< core::ecs::PhysicsSystem >( 0, & physics_manager );
	em.add_system< blue_sky::ecs::BalloonSystem >( 1, & physics_manager );

	auto* balloon = create_body( em, core::physics::CollisionGroup::Balloon );
	balloon->add_component< blue_sky::ecs::BalloonComponent >()->lift_force = 500.f;
	auto* player = create_body( em, core::physics::CollisionGroup::Player, 0.6f );

	em.update();	// 剛体を作る
	em.update();	// 当たりを調べ、風船がプレイヤーを持ち上げる

	const auto velocity = physics_manager.get_rigid_body_linear_velocity( player->get_component< core::ecs::RigidBodyComponent >()->handle );
	EXPECT_GT( velocity.y(), 100.f );	// 重力（1 フレームで -0.16 ほど）ではなく、浮力 500 で上へ
}
