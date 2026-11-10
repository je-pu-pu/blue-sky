#include "pch.h"

#include <blue_sky/ecs/system/CollisionEventSystem.h>
#include <blue_sky/ecs/component/CollisionTraitsComponent.h>
#include <blue_sky/ActiveObjectPhysics.h>

#include <core/ecs/EntityManager.h>
#include <core/physics/EntityTag.h>

#include <btBulletDynamicsCommon.h>

#include <functional>
#include <string>
#include <vector>

/**
 * 当たりの振り分け（ecs::CollisionEventSystem）と、剛体の Entity の番号のタグから性質を引くレイの判定（ClosestNotMe）のテスト
 *
 * 以前は userPointer の ActiveObject* をキャストして、その場で振り分けていた（ActiveObjectPhysics.cpp の on_collide）。
 * 振り分けの決まりは以前と同じ : 片方が地面なら、もう片方（生きていれば）が地面に当たる。両方が物体なら、両方とも生きていれば当たる
 */

using blue_sky::ecs::CollisionEventSystem;
using blue_sky::ecs::CollisionEventHandler;
using blue_sky::ecs::CollisionTraitsComponent;
using core::ecs::EntityId;
using core::ecs::EntityManager;

namespace
{

using EventType = CollisionEventSystem::EventType;
using Contact = CollisionEventSystem::Contact;

/// 当たりの性質を持つ Entity を作る
EntityId create_object( EntityManager& manager )
{
	auto* entity = manager.create_entity();
	entity->add_component< CollisionTraitsComponent >();
	return entity->get_id();
}

CollisionTraitsComponent* traits_of( EntityManager& manager, EntityId id )
{
	return manager.find( id )->get_component< CollisionTraitsComponent >();
}

/// 渡された当たりを文字にして記録する偽物（互換の層の代わり）
class RecordingHandler : public CollisionEventHandler
{
public:
	std::vector< std::string > log;
	std::function< void ( EntityId, EntityId ) > on_pair;	///< 当たったときに追加でする事

	void on_collide_with_ground( EntityId id ) override
	{
		log.push_back( "ground " + std::to_string( id ) );
	}

	void on_collide( EntityId a, EntityId b ) override
	{
		log.push_back( "pair " + std::to_string( a ) + " " + std::to_string( b ) );

		if ( on_pair )
		{
			on_pair( a, b );
		}
	}
};

CollisionEventSystem* add_system( EntityManager& manager )
{
	manager.add_system< CollisionEventSystem >( 0 );
	return manager.get_system< CollisionEventSystem >();
}

} // namespace

// ---- 振り分け ----

TEST( CollisionEventSystemTest, 片方が地面なら_もう片方が地面に当たる_順はどちらでも )
{
	EntityManager manager;
	auto* system = add_system( manager );
	const auto id = create_object( manager );

	const auto e1 = system->classify( { std::nullopt, id } );
	EXPECT_EQ( EventType::GROUND, e1.type );
	EXPECT_EQ( id, e1.a );

	const auto e2 = system->classify( { id, std::nullopt } );
	EXPECT_EQ( EventType::GROUND, e2.type );
	EXPECT_EQ( id, e2.a );
}

TEST( CollisionEventSystemTest, 両方とも地面なら何も起こさない )
{
	EntityManager manager;
	auto* system = add_system( manager );

	EXPECT_EQ( EventType::NONE, system->classify( { std::nullopt, std::nullopt } ).type );
}

TEST( CollisionEventSystemTest, 死んだ物体は地面にも物体にも当たらない )
{
	EntityManager manager;
	auto* system = add_system( manager );
	const auto dead = create_object( manager );
	const auto alive = create_object( manager );
	traits_of( manager, dead )->is_dead = true;

	EXPECT_EQ( EventType::NONE, system->classify( { std::nullopt, dead } ).type );
	EXPECT_EQ( EventType::NONE, system->classify( { dead, alive } ).type );
	EXPECT_EQ( EventType::NONE, system->classify( { alive, dead } ).type );
}

TEST( CollisionEventSystemTest, 二つの物体は集めた順のまま当たる )
{
	EntityManager manager;
	auto* system = add_system( manager );
	const auto a = create_object( manager );
	const auto b = create_object( manager );

	const auto e = system->classify( { b, a } );
	EXPECT_EQ( EventType::PAIR, e.type );
	EXPECT_EQ( b, e.a );
	EXPECT_EQ( a, e.b );
}

TEST( CollisionEventSystemTest, 消えた_Entity_のタグや性質を持たない_Entity_は何も起こさない )
{
	// 以前なら消えた物を指すポインタだった所。地面としても扱わない
	EntityManager manager;
	auto* system = add_system( manager );
	const auto vanished = create_object( manager );
	const auto alive = create_object( manager );
	const auto plain = manager.create_entity()->get_id();	// 当たりの性質を持たない
	manager.destroy_entity( vanished );

	EXPECT_EQ( EventType::NONE, system->classify( { std::nullopt, vanished } ).type );
	EXPECT_EQ( EventType::NONE, system->classify( { vanished, alive } ).type );
	EXPECT_EQ( EventType::NONE, system->classify( { alive, vanished } ).type );
	EXPECT_EQ( EventType::NONE, system->classify( { alive, plain } ).type );
	EXPECT_EQ( EventType::NONE, system->classify( { std::nullopt, 12345u } ).type );	// 作っていない番号
}

// ---- 集めて渡す ----

TEST( CollisionEventSystemTest, 集めた順に渡し_渡したら空になる )
{
	EntityManager manager;
	auto* system = add_system( manager );
	const auto a = create_object( manager );
	const auto b = create_object( manager );

	system->add_contact( Contact{ a, b } );
	system->add_contact( Contact{ std::nullopt, b } );
	system->add_contact( Contact{ std::nullopt, std::nullopt } );
	system->add_contact( Contact{ b, a } );
	ASSERT_EQ( 4u, system->debug_contact_count() );

	RecordingHandler handler;
	system->dispatch( handler );

	const std::vector< std::string > expected = {
		"pair " + std::to_string( a ) + " " + std::to_string( b ),
		"ground " + std::to_string( b ),
		"pair " + std::to_string( b ) + " " + std::to_string( a ),
	};
	EXPECT_EQ( expected, handler.log );
	EXPECT_EQ( 0u, system->debug_contact_count() );

	system->dispatch( handler );	// 2 度目は何も渡さない
	EXPECT_EQ( 3u, handler.log.size() );
}

TEST( CollisionEventSystemTest, 先に渡した当たりで死んだ物体は後の当たりを起こさない )
{
	// 以前もその場で is_dead を見ていた（例 : プレイヤーがメダルを取って消すと、同じフレームの残りの接点では取らない）
	EntityManager manager;
	auto* system = add_system( manager );
	const auto player = create_object( manager );
	const auto medal = create_object( manager );

	system->add_contact( Contact{ player, medal } );
	system->add_contact( Contact{ player, medal } );	// 同じ組の 2 つ目の接点
	system->add_contact( Contact{ std::nullopt, medal } );

	RecordingHandler handler;
	handler.on_pair = [ & ] ( EntityId, EntityId b ) { traits_of( manager, b )->is_dead = true; };	// メダルを取る（kill）

	system->dispatch( handler );

	ASSERT_EQ( 1u, handler.log.size() );
	EXPECT_EQ( "pair " + std::to_string( player ) + " " + std::to_string( medal ), handler.log[ 0 ] );
}

TEST( CollisionEventSystemTest, 剛体のタグから集め_性質を引く )
{
	EntityManager manager;
	auto* system = add_system( manager );
	const auto id = create_object( manager );

	btCollisionObject tagged;
	btCollisionObject ground;
	core::physics::set_entity_tag( & tagged, id );

	system->add_contact( & ground, & tagged );

	RecordingHandler handler;
	system->dispatch( handler );
	ASSERT_EQ( 1u, handler.log.size() );
	EXPECT_EQ( "ground " + std::to_string( id ), handler.log[ 0 ] );

	EXPECT_EQ( traits_of( manager, id ), CollisionEventSystem::find_traits( & manager, & tagged ) );
	EXPECT_EQ( nullptr, CollisionEventSystem::find_traits( & manager, & ground ) );
	EXPECT_EQ( nullptr, CollisionEventSystem::find_traits( nullptr, & tagged ) );
	EXPECT_FALSE( CollisionEventSystem::is_vanished( & manager, & tagged ) );
	EXPECT_FALSE( CollisionEventSystem::is_vanished( & manager, & ground ) );

	manager.destroy_entity( id );
	EXPECT_EQ( nullptr, CollisionEventSystem::find_traits( & manager, & tagged ) );
	EXPECT_TRUE( CollisionEventSystem::is_vanished( & manager, & tagged ) );
}

// ---- レイの判定（ClosestNotMe） ----

namespace
{

core::math::Transform at_height( float_t y )
{
	core::math::Transform t;
	t.set_identity();
	t.set_position( core::math::Vector( 0.f, y, 0.f ) );
	return t;
}

/// 上から下へレイを飛ばし、最初に当たった剛体を返す（当たらなければ nullptr）
const btCollisionObject* cast_down( const blue_sky::ActiveObjectPhysics& physics, const EntityManager& manager, const btRigidBody* me, bool include_soft_footing )
{
	const btVector3 from( 0.f, 100.f, 0.f );
	const btVector3 to( 0.f, -100.f, 0.f );

	blue_sky::ClosestNotMe callback( from, to, me, include_soft_footing, & manager );
	physics.ray_test( from, to, callback );

	return callback.hasHit() ? callback.m_collisionObject : nullptr;
}

} // namespace

TEST( CollisionEventSystemTest, レイは自分_死んだ物体_すり抜ける物体_柔らかい物体_消えた物体を通り抜け_地面に当たる )
{
	EntityManager manager;
	blue_sky::ActiveObjectPhysics physics;

	const core::math::Transform identity = at_height( 0.f );
	const core::math::Vector box( 1.f, 0.1f, 1.f );

	// 上から順に : 自分 → 死んだ物体 → すり抜ける物体 → 柔らかい物体（風船など）→ 消えた物体 → 地面（タグ無し）
	btRigidBody* me = physics.add_box_rigid_body( at_height( 50.f ), identity, box, 0.f );
	btRigidBody* dead = physics.add_box_rigid_body( at_height( 40.f ), identity, box, 0.f );
	btRigidBody* ghost = physics.add_box_rigid_body( at_height( 30.f ), identity, box, 0.f );
	btRigidBody* soft = physics.add_box_rigid_body( at_height( 20.f ), identity, box, 0.f );
	btRigidBody* vanished = physics.add_box_rigid_body( at_height( 10.f ), identity, box, 0.f );
	btRigidBody* ground = physics.add_box_rigid_body( at_height( 0.f ), identity, box, 0.f );

	const auto me_id = create_object( manager );
	const auto dead_id = create_object( manager );
	const auto ghost_id = create_object( manager );
	const auto soft_id = create_object( manager );
	const auto vanished_id = create_object( manager );

	traits_of( manager, dead_id )->is_dead = true;
	traits_of( manager, ghost_id )->is_ghost = true;
	traits_of( manager, soft_id )->is_hard = false;

	core::physics::set_entity_tag( me, me_id );
	core::physics::set_entity_tag( dead, dead_id );
	core::physics::set_entity_tag( ghost, ghost_id );
	core::physics::set_entity_tag( soft, soft_id );
	core::physics::set_entity_tag( vanished, vanished_id );
	manager.destroy_entity( vanished_id );

	// 柔らかい物体を足場に含めないなら、地面まで届く
	EXPECT_EQ( ground, cast_down( physics, manager, me, false ) );

	// 足場に含めるなら、柔らかい物体に当たる
	EXPECT_EQ( soft, cast_down( physics, manager, me, true ) );

	// 自分でなければ、自分（の位置にある物体）に当たる
	EXPECT_EQ( me, cast_down( physics, manager, nullptr, false ) );
}
