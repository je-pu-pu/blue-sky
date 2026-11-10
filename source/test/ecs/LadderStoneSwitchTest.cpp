#include "pch.h"

#include <blue_sky/ObjectFactory.h>
#include <blue_sky/GameObjectRef.h>
#include <blue_sky/ActiveObjectManager.h>
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

/// 描画なし・ActiveObject なしで工場を作る
struct Fixture
{
	blue_sky::ActiveObjectPhysics physics;
	EntityManager manager;
	ObjectFactory factory{ & manager, nullptr, & physics, nullptr };

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

/// 当たりを受け取る偽の物体（ActiveObject の代わり。GameObject の仮想関数が呼ばれた回数を数える）
class FakeObject : public blue_sky::GameObject
{
public:
	bool reacts = true;			///< 梯子に反応する・石を拾う（プレイヤー）か

	int ladder_count = 0;

	void restart() override { }
	void update() override { }
	float_t get_collision_width() const override { return 1.f; }
	float_t get_collision_height() const override { return 1.f; }
	float_t get_collision_depth() const override { return 1.f; }

	bool on_collide_with_ladder() override { ladder_count++; return reacts; }
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

	// 反応しない物体（敵など）: 知らせるが、覚えない（以前も梯子は Player との当たりでだけ音を鳴らした）
	FakeObject robot;
	robot.reacts = false;
	blue_sky::ActiveObjectManager::on_collide_with_entity_object( robot, 100, * f.entity( id ) );

	EXPECT_EQ( robot.ladder_count, 1 );
	EXPECT_FALSE( cache->cache.is_cached( 100 ) );

	// プレイヤー : 知らせて、覚える
	FakeObject player;
	blue_sky::ActiveObjectManager::on_collide_with_entity_object( player, 200, * f.entity( id ) );

	EXPECT_EQ( player.ladder_count, 1 );
	EXPECT_TRUE( cache->cache.is_cached( 200 ) );

	// 0.5 秒で忘れる（ContactCacheSystem が減らす）
	auto* system = f.manager.get_system< blue_sky::ecs::ContactCacheSystem >();
	ASSERT_NE( system, nullptr );

	system->update_caches( 0.3f );
	EXPECT_TRUE( cache->cache.is_cached( 200 ) );

	system->update_caches( 0.3f );
	EXPECT_FALSE( cache->cache.is_cached( 200 ) );

	// やり直すと忘れる
	blue_sky::ActiveObjectManager::on_collide_with_entity_object( player, 200, * f.entity( id ) );
	EXPECT_TRUE( cache->cache.is_cached( 200 ) );

	f.factory.restart();
	EXPECT_FALSE( cache->cache.is_cached( 200 ) );
}

// ---- 石 ----

namespace
{

/// 石を受け取る偽の物体（プレイヤー・敵・風船の代わり）。picks が true なら拾う（プレイヤー）
class FakeStoneReceiver : public FakeObject
{
public:
	bool picks = false;
	std::vector< GameObjectRef > stones;

	bool on_collide_with_stone( const GameObjectRef& stone ) override
	{
		stones.push_back( stone );
		return picks;
	}
};

} // namespace

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

TEST( LadderStoneSwitchTest, 石に当たると相手に石の窓口を渡し_拾われたら石は死ぬ_拾わない相手なら生きたまま )
{
	Fixture f;

	const auto id = f.create( "stone" );
	const auto* traits = f.entity( id )->get_component< blue_sky::ecs::CollisionTraitsComponent >();

	// 拾わない物体（敵・風船・スイッチ）: 石を受け取るが、石は生きたまま
	FakeStoneReceiver robot;
	blue_sky::ActiveObjectManager::on_collide_with_entity_object( robot, 100, * f.entity( id ) );

	ASSERT_EQ( robot.stones.size(), 1u );
	EXPECT_EQ( robot.stones[ 0 ].get_entity_id(), id );
	EXPECT_FALSE( traits->is_dead );

	// プレイヤー : 拾い、石は死んで描かれない
	FakeStoneReceiver player;
	player.picks = true;
	blue_sky::ActiveObjectManager::on_collide_with_entity_object( player, 200, * f.entity( id ) );

	ASSERT_EQ( player.stones.size(), 1u );
	EXPECT_TRUE( traits->is_dead );
	EXPECT_FALSE( f.entity( id )->get_component< blue_sky::ecs::ObjectRenderComponent >()->is_visible );
}

TEST( LadderStoneSwitchTest, 拾った石を窓口で投げると生き返り_置いた位置から投げた速度で飛ぶ )
{
	Fixture f;

	const auto id = f.create( "stone" );
	f.factory.restart();

	FakeStoneReceiver player;
	player.picks = true;
	blue_sky::ActiveObjectManager::on_collide_with_entity_object( player, 200, * f.entity( id ) );

	// 以前の Player::throw_stone と同じ手順（やり直す → 置く → 速度を設定する）
	GameObjectRef stone = player.stones.front();
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

/// スイッチを押す偽のプレイヤー（剛体を持ち、速度を設定できる）
class FakePresser : public FakeObject
{
	core::physics::RigidBody body_;

public:
	explicit FakePresser( blue_sky::ActiveObjectPhysics& physics )
	{
		blue_sky::Transform t;
		t.set_identity();
		blue_sky::Transform offset;
		offset.set_identity();

		body_ = physics.add_box_rigid_body( t, offset, Vector( 0.5f, 0.5f, 0.5f ), 1.f );
		set_rigid_body_view( body_.get() );
	}

	~FakePresser() { set_rigid_body_view( nullptr ); }

	bool presses = true;
	bool can_press_switch() const override { return presses; }
};

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

	FakePresser player( f.physics );

	// 遅い（0.5 未満）: 切り替わらないが、覚える
	player.set_velocity( Vector( 0.4f, 0.f, 0.f ) );
	blue_sky::ActiveObjectManager::on_collide_with_entity_object( player, 200, * f.entity( id ) );
	EXPECT_EQ( state_of( f, id ), blue_sky::ecs::SwitchComponent::State::OFF );

	// 速くても、覚えている間は切り替わらない
	player.set_velocity( Vector( 1.f, 0.f, 0.f ) );
	blue_sky::ActiveObjectManager::on_collide_with_entity_object( player, 200, * f.entity( id ) );
	EXPECT_EQ( state_of( f, id ), blue_sky::ecs::SwitchComponent::State::OFF );

	// 触れ続けると覚え直すので、0.5 秒触れずにいてから
	f.manager.get_system< blue_sky::ecs::ContactCacheSystem >()->update_caches( 0.6f );
	blue_sky::ActiveObjectManager::on_collide_with_entity_object( player, 200, * f.entity( id ) );
	EXPECT_EQ( state_of( f, id ), blue_sky::ecs::SwitchComponent::State::ON );

	// 押せない物体（敵など）は速くても切り替えない
	FakePresser robot( f.physics );
	robot.presses = false;
	robot.set_velocity( Vector( 1.f, 0.f, 0.f ) );
	blue_sky::ActiveObjectManager::on_collide_with_entity_object( robot, 300, * f.entity( id ) );
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
	blue_sky::ActiveObjectManager::on_collide_entity_objects( * f.entity( stone ), * f.entity( s ) );
	EXPECT_EQ( state_of( f, s ), blue_sky::ecs::SwitchComponent::State::OFF );

	// 速い石（どちらの順で渡しても同じ。覚えている間は変わらないので、忘れてから）
	f.manager.get_system< blue_sky::ecs::ContactCacheSystem >()->update_caches( 0.6f );
	f.factory.get_ref( stone ).set_velocity( Vector( 0.f, 0.f, 8.f ) );
	blue_sky::ActiveObjectManager::on_collide_entity_objects( * f.entity( s ), * f.entity( stone ) );
	EXPECT_EQ( state_of( f, s ), blue_sky::ecs::SwitchComponent::State::ON );

	blue_sky::ActiveObjectManager::on_collide_entity_objects( * f.entity( stone ), * f.entity( area ) );
	EXPECT_EQ( state_of( f, area ), blue_sky::ecs::SwitchComponent::State::OFF );

	// 石でない物体どうしは何もしない
	const auto wall = f.create( "static" );
	blue_sky::ActiveObjectManager::on_collide_entity_objects( * f.entity( wall ), * f.entity( s ) );
	EXPECT_EQ( state_of( f, s ), blue_sky::ecs::SwitchComponent::State::ON );
}
