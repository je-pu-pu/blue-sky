#include "pch.h"

#include <blue_sky/ecs/system/CollisionEventSystem.h>
#include <blue_sky/ecs/component/CollisionTraitsComponent.h>

#include <core/ecs/EntityManager.h>
#include <core/physics/PhysicsManager.h>

#include <functional>
#include <string>
#include <vector>

/**
 * 当たりの振り分け（ecs::CollisionEventSystem）と、剛体の Entity の番号のタグから性質を引くレイ・壁の判定の決まりのテスト
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

	// PhysicsManager の接触の判定が渡すタグ（地面はタグ無し）
	const std::optional< EntityId > tagged = id;
	const std::optional< EntityId > ground;

	system->add_contact( ground, tagged );

	RecordingHandler handler;
	system->dispatch( handler );
	ASSERT_EQ( 1u, handler.log.size() );
	EXPECT_EQ( "ground " + std::to_string( id ), handler.log[ 0 ] );

	EXPECT_EQ( traits_of( manager, id ), CollisionEventSystem::find_traits( & manager, tagged ) );
	EXPECT_EQ( nullptr, CollisionEventSystem::find_traits( & manager, ground ) );
	EXPECT_EQ( nullptr, CollisionEventSystem::find_traits( nullptr, tagged ) );
	EXPECT_FALSE( CollisionEventSystem::is_vanished( & manager, tagged ) );
	EXPECT_FALSE( CollisionEventSystem::is_vanished( & manager, ground ) );

	manager.destroy_entity( id );
	EXPECT_EQ( nullptr, CollisionEventSystem::find_traits( & manager, tagged ) );
	EXPECT_TRUE( CollisionEventSystem::is_vanished( & manager, tagged ) );
}
// ---- レイの判定（以前の ClosestNotMe。今は PhysicsManager::ray_test_closest と CollisionEventSystem::is_ray_target） ----

namespace
{

core::math::Transform at_height( float_t y )
{
	core::math::Transform t;
	t.set_identity();
	t.set_position( core::math::Vector( 0.f, y, 0.f ) );
	return t;
}

/// 本編の物体と同じ作り方の、動かない薄い箱
core::physics::RigidBody make_plate( core::physics::PhysicsManager& physics, float_t y )
{
	core::physics::RigidBodyCreateInfo info;
	info.shape_size = core::math::Vector( 1.f, 0.1f, 1.f );
	info.mass = 0.f;
	info.transform = at_height( y );
	info.offset = at_height( 0.f );
	info.friction = core::physics::RigidBodyCreateInfo::BulletDefaultFriction;
	info.share_shape = true;
	info.use_default_collision_filter = true;

	return physics.create_rigid_body( info );
}

/// 上から下へレイを飛ばし、最初に当たった所を返す（本編のプレイヤーの足場のレイと同じ決まり）
core::physics::RayHit cast_down( const core::physics::PhysicsManager& physics, const EntityManager& manager, const core::physics::RigidBody* me, bool include_soft_footing )
{
	return physics.ray_test_closest( core::math::Vector( 0.f, 100.f, 0.f ), core::math::Vector( 0.f, -100.f, 0.f ), me, nullptr,
		[ & manager, include_soft_footing ] ( std::optional< EntityId > tag ) { return CollisionEventSystem::is_ray_target( & manager, tag, include_soft_footing ); } );
}

} // namespace

TEST( CollisionEventSystemTest, レイは自分_死んだ物体_すり抜ける物体_柔らかい物体_消えた物体を通り抜け_地面に当たる )
{
	EntityManager manager;
	core::physics::PhysicsManager physics;

	// 上から順に : 自分 → 死んだ物体 → すり抜ける物体 → 柔らかい物体（風船など）→ 消えた物体 → 地面（タグ無し）
	auto me = make_plate( physics, 50.f );
	auto dead = make_plate( physics, 40.f );
	auto ghost = make_plate( physics, 30.f );
	auto soft = make_plate( physics, 20.f );
	auto vanished = make_plate( physics, 10.f );
	auto ground = make_plate( physics, 0.f );

	const auto me_id = create_object( manager );
	const auto dead_id = create_object( manager );
	const auto ghost_id = create_object( manager );
	const auto soft_id = create_object( manager );
	const auto vanished_id = create_object( manager );

	traits_of( manager, dead_id )->is_dead = true;
	traits_of( manager, ghost_id )->is_ghost = true;
	traits_of( manager, soft_id )->is_hard = false;

	me.set_entity_tag( me_id );
	dead.set_entity_tag( dead_id );
	ghost.set_entity_tag( ghost_id );
	soft.set_entity_tag( soft_id );
	vanished.set_entity_tag( vanished_id );
	manager.destroy_entity( vanished_id );

	// 柔らかい物体を足場に含めないなら、地面まで届く
	const auto to_ground = cast_down( physics, manager, & me, false );
	ASSERT_TRUE( to_ground.hit );
	EXPECT_FALSE( to_ground.entity.has_value() );
	EXPECT_NEAR( 0.1f, to_ground.point.y(), 1e-4f );

	// 足場に含めるなら、柔らかい物体に当たる
	EXPECT_EQ( std::optional< EntityId >( soft_id ), cast_down( physics, manager, & me, true ).entity );

	// 自分でなければ、自分（の位置にある物体）に当たる
	EXPECT_EQ( std::optional< EntityId >( me_id ), cast_down( physics, manager, nullptr, false ).entity );
}

TEST( CollisionEventSystemTest, 壁の判定は消えた物体と塞がない物体を数えず_地面と塞ぐ物体を数える )
{
	EntityManager manager;

	const auto block_id = create_object( manager );
	const auto open_id = create_object( manager );
	const auto vanished_id = create_object( manager );

	traits_of( manager, block_id )->is_block = true;
	traits_of( manager, open_id )->is_block = false;
	manager.destroy_entity( vanished_id );

	// 以前のプレイヤーの WithoutMeContactResultCallback と同じ決まり
	EXPECT_TRUE( CollisionEventSystem::is_block( & manager, std::nullopt ) );					// 地面・ステージの当たりの形
	EXPECT_TRUE( CollisionEventSystem::is_block( & manager, block_id ) );
	EXPECT_FALSE( CollisionEventSystem::is_block( & manager, open_id ) );
	EXPECT_FALSE( CollisionEventSystem::is_block( & manager, vanished_id ) );
	EXPECT_TRUE( CollisionEventSystem::is_block( & manager, manager.create_entity()->get_id() ) );	// 性質を持たない Entity は数える
}