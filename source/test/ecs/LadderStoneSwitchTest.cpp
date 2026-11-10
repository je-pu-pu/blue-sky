#include "pch.h"

#include <blue_sky/ObjectFactory.h>
#include <blue_sky/GameObjectRef.h>
#include <blue_sky/ecs/ContactRules.h>
#include <blue_sky/ecs/component/PlayerBehaviorComponent.h>
#include <blue_sky/ActiveObjectPhysics.h>

#include <blue_sky/ecs/EntityObject.h>
#include <blue_sky/ecs/component/CollisionTraitsComponent.h>
#include <blue_sky/ecs/component/ContactCacheComponent.h>
#include <blue_sky/ecs/component/ObjectRenderComponent.h>
#include <blue_sky/ecs/component/PhysicsBodyComponent.h>
#include <blue_sky/ecs/component/SimpleObjectComponent.h>
#include <blue_sky/ecs/component/StaticObjectComponent.h>
#include <blue_sky/ecs/component/PickupComponent.h>
#include <blue_sky/ecs/system/ContactCacheSystem.h>
#include <blue_sky/ecs/system/SpinSystem.h>
#include <blue_sky/ecs/system/SwitchSystem.h>
#include <blue_sky/ecs/component/SwitchComponent.h>

#include <core/ecs/EntityManager.h>
#include <core/physics/RigidBody.h>

#include <btBulletDynamicsCommon.h>

#include <cmath>
#include <optional>
#include <string>
#include <vector>

/**
 * 段階 4 の後半 : 梯子・石・スイッチを ECS だけの物体にしたときの、部品と当たりの振る舞いのテスト
 *
 * 描画（GraphicsManager）・音は無しで作る。物理の世界は本物（ActiveObjectPhysics）を使う
 */

using blue_sky::ObjectFactory;
using blue_sky::GameObjectRef;
using core::ecs::EntityManager;
using core::ecs::EntityId;
using core::Vector;

namespace
{

/// 描画なしで工場を作る
struct Fixture
{
	blue_sky::ActiveObjectPhysics physics;
	EntityManager manager;
	ObjectFactory factory{ & manager, & physics, nullptr };

	/// プレイヤー（ECS だけの物体）を作り、剛体も作る（段階 6 までは GameObject の仮想関数を実装した偽物を使っていた）
	EntityId create_player()
	{
		const EntityId id = factory.create_player();
		factory.add_player_rigid_body( id );
		return id;
	}

	blue_sky::ecs::PlayerBehaviorComponent& player_of( EntityId id ) { return * entity( id )->get_component< blue_sky::ecs::PlayerBehaviorComponent >(); }

	~Fixture()
	{
		// 剛体の持ち主（部品）を、物理の世界より先に消す
		factory.clear();
		manager.clear();
	}

	core::ecs::Entity* entity( EntityId id ) { return manager.find( id ); }

	EntityId create( const char* kind )
	{
		auto id = factory.create_object( kind );
		EXPECT_TRUE( id.has_value() );
		return * id;
	}
};

bool is_near( const Vector& a, const Vector& b, float_t e = 0.0001f )
{
	return std::abs( a.x() - b.x() ) < e && std::abs( a.y() - b.y() ) < e && std::abs( a.z() - b.z() ) < e;
}

} // namespace

// ---- 梯子 ----

TEST( LadderStoneSwitchTest, 梯子は硬くない印付きの物体で_重力が無く_やり直すと動かなくなる )
{
	Fixture f;

	const auto id = f.create( "ladder" );
	auto* e = f.entity( id );

	ASSERT_NE( e->get_component< blue_sky::ecs::LadderComponent >(), nullptr );
	ASSERT_NE( e->get_component< blue_sky::ecs::ContactCacheComponent >(), nullptr );
	EXPECT_EQ( e->get_component< blue_sky::ecs::StaticObjectComponent >(), nullptr );
	EXPECT_EQ( e->get_component< blue_sky::ecs::PickupComponent >(), nullptr );

	// 以前の Ladder : 0.4 x 5 x 0.2・硬くない（当たりの応答なし・重力 0・壁にならない）・作ったときの質量 1、やり直すと 0
	const auto* traits = e->get_component< blue_sky::ecs::CollisionTraitsComponent >();
	EXPECT_FALSE( traits->is_hard );
	EXPECT_FALSE( traits->is_block );
	EXPECT_FALSE( traits->is_ghost );

	const auto* body = e->get_component< blue_sky::ecs::PhysicsBodyComponent >();
	EXPECT_FLOAT_EQ( body->height_offset, 2.5f );
	EXPECT_NE( body->body->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE, 0 );
	EXPECT_FLOAT_EQ( body->body->getInvMass(), 1.f );
	EXPECT_TRUE( is_near( e->get_component< blue_sky::ecs::SimpleObjectComponent >()->default_gravity, Vector::Zero ) );

	f.factory.restart();

	EXPECT_FLOAT_EQ( body->body->getInvMass(), 0.f );
}

TEST( LadderStoneSwitchTest, 梯子に触れると相手に知らせ_反応した物体を_0_5_秒覚え_やり直すと忘れる )
{
	Fixture f;

	const auto id = f.create( "ladder" );
	auto* cache = f.entity( id )->get_component< blue_sky::ecs::ContactCacheComponent >();

	// プレイヤーでない物体（敵）: 覚えない（以前も梯子は Player との当たりでだけ音を鳴らした）
	const auto robot = f.create( "robot" );
	blue_sky::ecs::contact_rules::on_collide( * f.entity( robot ), * f.entity( id ) );

	EXPECT_FALSE( cache->cache.is_cached( robot ) );

	// プレイヤー : 梯子につかまり、梯子は覚える
	const auto player = f.create_player();
	blue_sky::ecs::contact_rules::on_collide( * f.entity( player ), * f.entity( id ) );

	EXPECT_TRUE( f.player_of( player ).is_on_ladder );
	EXPECT_TRUE( cache->cache.is_cached( player ) );

	// 0.5 秒で忘れる（ContactCacheSystem が減らす）
	auto* system = f.manager.get_system< blue_sky::ecs::ContactCacheSystem >();
	ASSERT_NE( system, nullptr );

	system->update_caches( 0.3f );
	EXPECT_TRUE( cache->cache.is_cached( player ) );

	system->update_caches( 0.3f );
	EXPECT_FALSE( cache->cache.is_cached( player ) );

	// やり直すと忘れる
	blue_sky::ecs::contact_rules::on_collide( * f.entity( id ), * f.entity( player ) );
	EXPECT_TRUE( cache->cache.is_cached( player ) );

	f.factory.restart();
	EXPECT_FALSE( cache->cache.is_cached( player ) );
}

// ---- 石 ----

TEST( LadderStoneSwitchTest, 石は硬いが壁にならない投げられる物で_重力は_8_やり直すと貫通を防ぐ設定になる )
{
	Fixture f;

	const auto id = f.create( "stone" );
	auto* e = f.entity( id );

	ASSERT_NE( e->get_component< blue_sky::ecs::ThrowableComponent >(), nullptr );
	ASSERT_NE( e->get_component< blue_sky::ecs::SpinComponent >(), nullptr );
	EXPECT_EQ( e->get_component< blue_sky::ecs::PickupComponent >(), nullptr );

	// 以前の Stone : 0.2 x 0.2 x 0.2・硬い・壁にならない・重力 ( 0, -8, 0 )・質量 1
	const auto* traits = e->get_component< blue_sky::ecs::CollisionTraitsComponent >();
	EXPECT_TRUE( traits->is_hard );
	EXPECT_FALSE( traits->is_block );

	const auto* body = e->get_component< blue_sky::ecs::PhysicsBodyComponent >()->body;
	EXPECT_EQ( body->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE, 0 );
	EXPECT_FLOAT_EQ( body->getInvMass(), 1.f );

	f.factory.restart();

	EXPECT_TRUE( is_near( Vector( body->getGravity() ), Vector( 0.f, -8.f, 0.f ) ) );
	EXPECT_FLOAT_EQ( body->getCcdSweptSphereRadius(), 0.2f );
	EXPECT_FLOAT_EQ( body->getCcdMotionThreshold(), 0.2f );
}

TEST( LadderStoneSwitchTest, 石は毎フレーム_1_度ずつ回る )
{
	Fixture f;

	const auto id = f.create( "stone" );
	f.factory.restart();

	auto* system = f.manager.get_system< blue_sky::ecs::SpinSystem >();
	ASSERT_NE( system, nullptr );

	// 以前の Stone::update : set_direction_degree( get_direction_degree() + 1.f )（経った時間によらない）
	system->update_spins( 0.5f );
	system->update_spins( 0.001f );

	EXPECT_FLOAT_EQ( f.entity( id )->get_component< blue_sky::ecs::SimpleObjectComponent >()->direction_degree, 2.f );
}

TEST( LadderStoneSwitchTest, プレイヤーが石に当たると石の窓口を持って拾い_石は死ぬ_拾わない物なら生きたまま )
{
	Fixture f;

	const auto id = f.create( "stone" );
	const auto* traits = f.entity( id )->get_component< blue_sky::ecs::CollisionTraitsComponent >();

	// 拾わない物体（動かない物）: 石は生きたまま
	const auto wall = f.create( "static" );
	blue_sky::ecs::contact_rules::on_collide( * f.entity( wall ), * f.entity( id ) );

	EXPECT_FALSE( traits->is_dead );

	// プレイヤー : 拾い（石の窓口を持つ）、石は死んで描かれない
	const auto player = f.create_player();
	blue_sky::ecs::contact_rules::on_collide( * f.entity( player ), * f.entity( id ) );

	ASSERT_EQ( f.player_of( player ).stone_list.size(), 1u );
	EXPECT_EQ( f.player_of( player ).stone_list.front().get_entity_id(), id );
	EXPECT_TRUE( traits->is_dead );
	EXPECT_FALSE( f.entity( id )->get_component< blue_sky::ecs::ObjectRenderComponent >()->is_visible );
}

TEST( LadderStoneSwitchTest, 拾った石を窓口で投げると生き返り_置いた位置から投げた速度で飛ぶ )
{
	Fixture f;

	const auto id = f.create( "stone" );
	f.factory.restart();

	const auto player = f.create_player();
	blue_sky::ecs::contact_rules::on_collide( * f.entity( id ), * f.entity( player ) );

	// 以前の Player::throw_stone と同じ手順（やり直す → 置く → 速度を設定する）
	GameObjectRef stone = f.player_of( player ).stone_list.front();
	stone.restart();
	stone.set_location( Vector( 1.f, 2.f, 3.f ) );
	stone.set_velocity( Vector( 0.f, 0.f, 8.f ) );

	EXPECT_FALSE( f.entity( id )->get_component< blue_sky::ecs::CollisionTraitsComponent >()->is_dead );
	EXPECT_TRUE( is_near( stone.get_location(), Vector( 1.f, 2.f, 3.f ) ) );
	EXPECT_TRUE( is_near( stone.get_velocity(), Vector( 0.f, 0.f, 8.f ) ) );

	// 敵が使う「こちらへ飛んでくるか」
	EXPECT_TRUE( stone.is_moving_to( Vector( 1.f, 2.f, 10.f ) ) );
	EXPECT_FALSE( stone.is_moving_to( Vector( 1.f, 2.f, -10.f ) ) );
}

// ---- スイッチ ----

namespace
{

blue_sky::ecs::SwitchComponent::State state_of( Fixture& f, EntityId id )
{
	return * f.manager.get_system< blue_sky::ecs::SwitchSystem >()->get_state( id );
}

} // namespace

TEST( LadderStoneSwitchTest, スイッチは硬い箱で_やり直すと切に戻り_質量_100_摩擦_1_5_回らない )
{
	Fixture f;

	const auto id = f.create( "switch" );
	auto* e = f.entity( id );

	ASSERT_NE( e->get_component< blue_sky::ecs::SwitchComponent >(), nullptr );
	ASSERT_NE( e->get_component< blue_sky::ecs::ContactCacheComponent >(), nullptr );

	const auto* traits = e->get_component< blue_sky::ecs::CollisionTraitsComponent >();
	EXPECT_TRUE( traits->is_hard );
	EXPECT_TRUE( traits->is_block );
	EXPECT_FALSE( traits->is_ghost );

	const auto* body = e->get_component< blue_sky::ecs::PhysicsBodyComponent >()->body;
	EXPECT_FLOAT_EQ( body->getInvMass(), 1.f );		// 作ったときは 1（以前と同じ）

	e->get_component< blue_sky::ecs::SwitchComponent >()->state = blue_sky::ecs::SwitchComponent::State::BROKEN;
	f.factory.restart();

	EXPECT_EQ( state_of( f, id ), blue_sky::ecs::SwitchComponent::State::OFF );
	EXPECT_FLOAT_EQ( body->getInvMass(), 1.f / 100.f );
	EXPECT_FLOAT_EQ( body->getFriction(), 1.5f );
	EXPECT_TRUE( is_near( Vector( body->getAngularFactor() ), Vector::Zero ) );
}

TEST( LadderStoneSwitchTest, 空間スイッチはすり抜ける箱で_足元と剛体の中心が同じ_重力が無い )
{
	Fixture f;

	const auto id = f.factory.create_area_switch( 6.f, 2.f, 4.f );
	auto* e = f.entity( id );

	const auto* s = e->get_component< blue_sky::ecs::SwitchComponent >();
	ASSERT_NE( s, nullptr );
	EXPECT_FALSE( s->is_switched_by_stone );
	EXPECT_FALSE( s->has_feedback );

	const auto* traits = e->get_component< blue_sky::ecs::CollisionTraitsComponent >();
	EXPECT_TRUE( traits->is_ghost );
	EXPECT_FALSE( traits->is_hard );
	EXPECT_FALSE( traits->is_block );

	const auto* body = e->get_component< blue_sky::ecs::PhysicsBodyComponent >();
	EXPECT_FLOAT_EQ( body->height_offset, 0.f );
	EXPECT_NE( body->body->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE, 0 );
	EXPECT_TRUE( is_near( e->get_component< blue_sky::ecs::SimpleObjectComponent >()->default_gravity, Vector::Zero ) );
}

TEST( LadderStoneSwitchTest, スイッチの切り替えで入と切のイベントを加えた順に呼び_壊れると変わらない )
{
	Fixture f;

	const auto id = f.create( "switch" );
	auto* system = f.manager.get_system< blue_sky::ecs::SwitchSystem >();

	std::vector< std::string > called;
	f.factory.get_ref( id ).add_event_handler( "turn-on", [ & ] { called.push_back( "on-1" ); } );
	f.factory.get_ref( id ).add_event_handler( "turn-on", [ & ] { called.push_back( "on-2" ); } );
	f.factory.get_ref( id ).add_event_handler( "turn-off", [ & ] { called.push_back( "off" ); } );

	EXPECT_TRUE( system->do_switch( id ) );
	EXPECT_EQ( state_of( f, id ), blue_sky::ecs::SwitchComponent::State::ON );
	EXPECT_EQ( called, ( std::vector< std::string >{ "on-1", "on-2" } ) );

	EXPECT_TRUE( system->do_switch( id ) );
	EXPECT_EQ( state_of( f, id ), blue_sky::ecs::SwitchComponent::State::OFF );
	EXPECT_EQ( called.back(), "off" );

	// 壊すと以後は切り替わらない（イベントも呼ばない）
	EXPECT_TRUE( system->do_break( id ) );
	EXPECT_FALSE( system->do_break( id ) );
	EXPECT_FALSE( system->do_switch( id ) );
	EXPECT_EQ( state_of( f, id ), blue_sky::ecs::SwitchComponent::State::BROKEN );
	EXPECT_EQ( called.size(), 3u );
}

TEST( LadderStoneSwitchTest, ステージのアクションの入と切と壊すはイベントを呼ばない_ほかのアクションは受け付けない )
{
	Fixture f;

	const auto id = f.create( "switch" );

	int called = 0;
	GameObjectRef s = f.factory.get_ref( id );
	s.add_event_handler( "turn-on", [ & ] { called++; } );
	s.add_event_handler( "turn-off", [ & ] { called++; } );

	s.action( "turn-on" );
	EXPECT_EQ( state_of( f, id ), blue_sky::ecs::SwitchComponent::State::ON );

	s.action( "turn-off" );
	EXPECT_EQ( state_of( f, id ), blue_sky::ecs::SwitchComponent::State::OFF );

	s.action( "set_visible 0 0" );	// 以前の BaseSwitch::action も受け付けなかった（今はログに残す）
	EXPECT_TRUE( f.entity( id )->get_component< blue_sky::ecs::ObjectRenderComponent >()->is_visible );

	s.action( "break" );
	EXPECT_EQ( state_of( f, id ), blue_sky::ecs::SwitchComponent::State::BROKEN );

	EXPECT_EQ( called, 0 );
}

TEST( LadderStoneSwitchTest, イベントを持たない物体はイベントを受け取らない )
{
	Fixture f;

	const auto id = f.create( "ladder" );

	int called = 0;
	f.factory.get_ref( id ).add_event_handler( "turn-on", [ & ] { called++; } );

	EXPECT_EQ( f.manager.get_system< blue_sky::ecs::SwitchSystem >()->get_state( id ), std::nullopt );
	EXPECT_EQ( called, 0 );
}

TEST( LadderStoneSwitchTest, プレイヤーが速く触れると切り替わり_0_5_秒は同じプレイヤーで切り替わらない )
{
	Fixture f;

	const auto id = f.create( "switch" );
	f.factory.restart();

	const auto player = f.create_player();
	GameObjectRef p = f.factory.get_ref( player );

	// 遅い（0.5 未満）: 切り替わらないが、覚える
	p.set_velocity( Vector( 0.4f, 0.f, 0.f ) );
	blue_sky::ecs::contact_rules::on_collide( * f.entity( player ), * f.entity( id ) );
	EXPECT_EQ( state_of( f, id ), blue_sky::ecs::SwitchComponent::State::OFF );

	// 速くても、覚えている間は切り替わらない
	p.set_velocity( Vector( 1.f, 0.f, 0.f ) );
	blue_sky::ecs::contact_rules::on_collide( * f.entity( id ), * f.entity( player ) );
	EXPECT_EQ( state_of( f, id ), blue_sky::ecs::SwitchComponent::State::OFF );

	// 触れ続けると覚え直すので、0.5 秒触れずにいてから
	f.manager.get_system< blue_sky::ecs::ContactCacheSystem >()->update_caches( 0.6f );
	blue_sky::ecs::contact_rules::on_collide( * f.entity( player ), * f.entity( id ) );
	EXPECT_EQ( state_of( f, id ), blue_sky::ecs::SwitchComponent::State::ON );

	// プレイヤーでない物体（敵）は速くても切り替えない
	const auto robot = f.create( "robot" );
	f.factory.get_ref( robot ).set_velocity( Vector( 1.f, 0.f, 0.f ) );
	f.manager.get_system< blue_sky::ecs::ContactCacheSystem >()->update_caches( 0.6f );
	blue_sky::ecs::contact_rules::on_collide( * f.entity( robot ), * f.entity( id ) );
	EXPECT_EQ( state_of( f, id ), blue_sky::ecs::SwitchComponent::State::ON );
}

TEST( LadderStoneSwitchTest, 速く飛ぶ石はスイッチを切り替え_空間スイッチは石では変わらない )
{
	Fixture f;

	const auto s = f.create( "switch" );
	const auto area = f.factory.create_area_switch( 2.f, 2.f, 2.f );
	const auto stone = f.create( "stone" );
	f.factory.restart();

	// 遅い石では変わらない
	f.factory.get_ref( stone ).set_velocity( Vector( 0.f, 0.f, 0.1f ) );
	blue_sky::ecs::contact_rules::on_collide( * f.entity( stone ), * f.entity( s ) );
	EXPECT_EQ( state_of( f, s ), blue_sky::ecs::SwitchComponent::State::OFF );

	// 速い石（どちらの順で渡しても同じ。覚えている間は変わらないので、忘れてから）
	f.manager.get_system< blue_sky::ecs::ContactCacheSystem >()->update_caches( 0.6f );
	f.factory.get_ref( stone ).set_velocity( Vector( 0.f, 0.f, 8.f ) );
	blue_sky::ecs::contact_rules::on_collide( * f.entity( s ), * f.entity( stone ) );
	EXPECT_EQ( state_of( f, s ), blue_sky::ecs::SwitchComponent::State::ON );

	blue_sky::ecs::contact_rules::on_collide( * f.entity( stone ), * f.entity( area ) );
	EXPECT_EQ( state_of( f, area ), blue_sky::ecs::SwitchComponent::State::OFF );

	// 石でない物体どうしは何もしない
	const auto wall = f.create( "static" );
	blue_sky::ecs::contact_rules::on_collide( * f.entity( wall ), * f.entity( s ) );
	EXPECT_EQ( state_of( f, s ), blue_sky::ecs::SwitchComponent::State::ON );
}
