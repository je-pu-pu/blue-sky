#include "pch.h"

#include <blue_sky/ObjectFactory.h>
#include <blue_sky/GameObjectRef.h>
#include <blue_sky/ecs/ContactRules.h>
#include <blue_sky/ActiveObjectPhysics.h>

#include <blue_sky/ecs/EntityObject.h>
#include <blue_sky/ecs/Flicker.h>
#include <blue_sky/ecs/component/BalloonHoldComponent.h>
#include <blue_sky/ecs/component/CollisionTraitsComponent.h>
#include <blue_sky/ecs/component/ObjectRenderComponent.h>
#include <blue_sky/ecs/component/PhysicsBodyComponent.h>
#include <blue_sky/ecs/component/SimpleObjectComponent.h>
#include <blue_sky/ecs/component/PickupComponent.h>
#include <blue_sky/ecs/system/BalloonHoldSystem.h>
#include <blue_sky/ecs/component/GirlComponent.h>
#include <blue_sky/ecs/component/FlickerComponent.h>
#include <blue_sky/ecs/system/GirlSystem.h>
#include <blue_sky/ecs/component/RobotBehaviorComponent.h>
#include <blue_sky/ecs/component/PatrolComponent.h>
#include <blue_sky/ecs/system/RobotBehaviorSystem.h>
#include <blue_sky/ecs/component/PlayerBehaviorComponent.h>
#include <blue_sky/ecs/system/PlayerBehaviorSystem.h>

#include <core/ecs/EntityManager.h>

#include <btBulletDynamicsCommon.h>

#include <cmath>
#include <optional>

/**
 * 段階 5 : 風船・少女・敵を ECS だけの物体にしたときの、部品と振る舞いと当たりのテスト
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
	ObjectFactory factory{ & manager, & physics, nullptr };

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

/// プレイヤー（ECS だけの物体。剛体なし）を作って (1, 2, 3) に置く。段階 6 までは窓口（BalloonHolder・ChaseTarget）を実装した偽物を使っていた
EntityId create_player( Fixture& f )
{
	const EntityId id = f.factory.create_player();
	f.factory.get_ref( id ).set_location( Vector( 1.f, 2.f, 3.f ) );
	return id;
}

blue_sky::ecs::PlayerBehaviorComponent& player_of( Fixture& f, EntityId id )
{
	return * f.entity( id )->get_component< blue_sky::ecs::PlayerBehaviorComponent >();
}

} // namespace

// ---- 風船 ----

TEST( BalloonGirlRobotTest, 風船は硬くない着地しても死なない足場で_重力が無く_足元と剛体の中心が同じ )
{
	Fixture f;

	const auto id = f.create( "balloon" );
	auto* e = f.entity( id );

	ASSERT_NE( e->get_component< blue_sky::ecs::BalloonHoldComponent >(), nullptr );
	EXPECT_EQ( e->get_component< blue_sky::ecs::PickupComponent >(), nullptr );

	// 以前の Balloon : 1.5 x 1.5 x 1.5・硬くない・壁にならない・着地しても死なない足場・高さのずれ 0・質量 1
	const auto* traits = e->get_component< blue_sky::ecs::CollisionTraitsComponent >();
	EXPECT_FALSE( traits->is_hard );
	EXPECT_FALSE( traits->is_block );
	EXPECT_TRUE( traits->is_safe_footing );

	const auto* body = e->get_component< blue_sky::ecs::PhysicsBodyComponent >();
	EXPECT_FLOAT_EQ( body->height_offset, 0.f );
	EXPECT_NE( body->body->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE, 0 );
	EXPECT_FLOAT_EQ( body->body->getInvMass(), 1.f );

	f.factory.restart();
	EXPECT_TRUE( is_near( Vector( body->body->getGravity() ), Vector::Zero ) );
}

TEST( BalloonGirlRobotTest, つかまれていない風船は始めの位置でゆらぎ_1_フレームに_1_度回る )
{
	Fixture f;

	const auto id = f.create( "balloon" );
	f.factory.get_ref( id ).set_start_location( 3.f, 4.f, 5.f );
	f.factory.restart();

	auto* system = f.manager.get_system< blue_sky::ecs::BalloonHoldSystem >();
	ASSERT_NE( system, nullptr );

	system->update_balloons( 2.f );
	system->update_balloons( 2.5f );

	// 以前の Balloon::update : update_location_by_flicker( get_start_location(), 0.5 )・set_direction_degree( 向き + 1 )。種は Entity の番号
	const float_t offset = blue_sky::ecs::flicker::height_offset( static_cast< unsigned int >( id ), 2.5f, 0.5f );
	EXPECT_TRUE( is_near( f.factory.get_ref( id ).get_location(), Vector( 3.f, 4.f + offset, 5.f ) ) );
	EXPECT_FLOAT_EQ( f.factory.get_ref( id ).get_direction_degree(), 2.f );
}

TEST( BalloonGirlRobotTest, 風船に触れると相手に知らせ_つかんだら手元に付いて行き_つかまれた風船は相手に知らせない )
{
	Fixture f;

	const auto id = f.create( "balloon" );
	f.factory.restart();

	const EntityId holder_id = create_player( f );
	auto& player = player_of( f, holder_id );

	// 揺らぎ中のプレイヤー : つかまない
	player.is_flickering = true;
	blue_sky::ecs::contact_rules::on_collide( * f.entity( holder_id ), * f.entity( id ) );

	EXPECT_EQ( player.balloon_sequence_count, 0 );
	EXPECT_FALSE( f.entity( id )->get_component< blue_sky::ecs::BalloonHoldComponent >()->is_held() );

	// プレイヤー : つかむ（組の向きによらない）
	player.is_flickering = false;
	blue_sky::ecs::contact_rules::on_collide( * f.entity( id ), * f.entity( holder_id ) );

	EXPECT_EQ( player.balloon_sequence_count, 1 );
	EXPECT_EQ( player.balloon, id );
	EXPECT_EQ( player.action_mode, blue_sky::ecs::PlayerBehaviorComponent::ActionMode::BALLOON );
	EXPECT_TRUE( f.entity( id )->get_component< blue_sky::ecs::BalloonHoldComponent >()->is_held() );

	// つかまれた風船は、もう知らせない（以前は Player::on_collide_with( Balloon* ) が balloon->get_player() を見て戻っていた）
	blue_sky::ecs::contact_rules::on_collide( * f.entity( holder_id ), * f.entity( id ) );
	EXPECT_EQ( player.balloon_sequence_count, 1 );

	// 手元（プレイヤーの位置 + 前 0.5 - 右 0.25 + 上 1.75。向き 0 なら前は +Z・右は +X）に置かれ、向きは 0
	f.factory.get_ref( id ).set_direction_degree( 30.f );
	f.manager.get_system< blue_sky::ecs::BalloonHoldSystem >()->update_balloon( id, 1.f );

	EXPECT_TRUE( is_near( f.factory.get_ref( id ).get_location(), Vector( 0.75f, 3.75f, 3.5f ), 0.001f ) );
	EXPECT_FLOAT_EQ( f.factory.get_ref( id ).get_direction_degree(), 0.f );
	EXPECT_FALSE( blue_sky::ecs::EntityObject( f.entity( id ) ).is_dead() );
}

TEST( BalloonGirlRobotTest, つかんだ物体が別の風船を持つか_風船の動作が終わると_風船は割れて下へ退く )
{
	Fixture f;

	const auto first = f.create( "balloon" );
	const auto second = f.create( "balloon" );
	f.factory.restart();

	const EntityId holder_id = create_player( f );
	auto* system = f.manager.get_system< blue_sky::ecs::BalloonHoldSystem >();

	blue_sky::ecs::contact_rules::on_collide( * f.entity( holder_id ), * f.entity( first ) );
	blue_sky::ecs::contact_rules::on_collide( * f.entity( holder_id ), * f.entity( second ) );

	system->update_balloons( 1.f );

	// 前の風船は割れて (0, -100, 0) へ（以前の Balloon::kill）。今の風船は手元
	EXPECT_TRUE( blue_sky::ecs::EntityObject( f.entity( first ) ).is_dead() );
	EXPECT_TRUE( is_near( f.factory.get_ref( first ).get_location(), Vector( 0.f, -100.f, 0.f ) ) );
	EXPECT_FALSE( blue_sky::ecs::EntityObject( f.entity( second ) ).is_dead() );

	// 動作が終わると、今の風船も割れる
	player_of( f, holder_id ).action_mode = blue_sky::ecs::PlayerBehaviorComponent::ActionMode::NONE;
	system->update_balloons( 1.1f );

	EXPECT_TRUE( blue_sky::ecs::EntityObject( f.entity( second ) ).is_dead() );
	EXPECT_FALSE( f.entity( second )->get_component< blue_sky::ecs::ObjectRenderComponent >()->is_visible );

	// やり直すと、つかまれていない・生きている・見える
	f.factory.restart();

	EXPECT_FALSE( f.entity( second )->get_component< blue_sky::ecs::BalloonHoldComponent >()->is_held() );
	EXPECT_FALSE( blue_sky::ecs::EntityObject( f.entity( second ) ).is_dead() );
	EXPECT_TRUE( f.entity( second )->get_component< blue_sky::ecs::ObjectRenderComponent >()->should_draw_mesh() );
}

TEST( BalloonGirlRobotTest, 風船の動作が終わりかけの間は_つかまれた風船が_1_秒に_6_回点滅する )
{
	Fixture f;

	const auto id = f.create( "balloon" );
	f.factory.restart();

	const EntityId holder_id = create_player( f );
	auto* system = f.manager.get_system< blue_sky::ecs::BalloonHoldSystem >();
	const auto* render = f.entity( id )->get_component< blue_sky::ecs::ObjectRenderComponent >();

	blue_sky::ecs::contact_rules::on_collide( * f.entity( holder_id ), * f.entity( id ) );

	// 終わりかけでなければ隠さない
	system->update_balloon( id, 0.1f );
	EXPECT_FALSE( render->is_hidden );

	// 終わりかけ : 1/6 秒ごとの前半は見え、後半は隠れる（以前の is_visible_in_blink( 6 )）
	player_of( f, holder_id ).is_action_pre_finish = true;

	system->update_balloon( id, 1.f / 6.f * 3.f + 0.01f );
	EXPECT_FALSE( render->is_hidden );
	EXPECT_TRUE( render->should_draw_mesh() );

	system->update_balloon( id, 1.f / 6.f * 3.f + 0.12f );
	EXPECT_TRUE( render->is_hidden );
	EXPECT_FALSE( render->should_draw_mesh() );
	EXPECT_FALSE( render->should_draw_line() );
	EXPECT_FALSE( render->should_write_constants() );
	EXPECT_TRUE( render->is_mesh_visible );	// 描くかどうかの値そのものは変えない
}

TEST( BalloonGirlRobotTest, つかんだ物体が消えると風船は割れる )
{
	Fixture f;

	const auto id = f.create( "balloon" );
	f.factory.restart();

	const EntityId holder_id = create_player( f );

	blue_sky::ecs::contact_rules::on_collide( * f.entity( holder_id ), * f.entity( id ) );
	ASSERT_TRUE( f.entity( id )->get_component< blue_sky::ecs::BalloonHoldComponent >()->is_held() );

	f.manager.destroy_entity( holder_id );
	f.manager.get_system< blue_sky::ecs::BalloonHoldSystem >()->update_balloon( id, 1.f );

	EXPECT_TRUE( blue_sky::ecs::EntityObject( f.entity( id ) ).is_dead() );
}

TEST( BalloonGirlRobotTest, 石が当たると風船は割れ_石は拾われず生きたまま )
{
	Fixture f;

	const auto balloon = f.create( "balloon" );
	const auto stone = f.create( "stone" );
	f.factory.restart();

	// 以前は Stone::on_collide_with( GameObject* ) → Balloon::on_collide_with_stone（割れて音を鳴らし、石は拾わない）。組の向きによらない
	blue_sky::ecs::contact_rules::on_collide( * f.entity( stone ), * f.entity( balloon ) );

	EXPECT_TRUE( blue_sky::ecs::EntityObject( f.entity( balloon ) ).is_dead() );
	EXPECT_TRUE( is_near( f.factory.get_ref( balloon ).get_location(), Vector( 0.f, -100.f, 0.f ) ) );
	EXPECT_FALSE( blue_sky::ecs::EntityObject( f.entity( stone ) ).is_dead() );

	f.factory.restart();
	blue_sky::ecs::contact_rules::on_collide( * f.entity( balloon ), * f.entity( stone ) );

	EXPECT_TRUE( blue_sky::ecs::EntityObject( f.entity( balloon ) ).is_dead() );
}

TEST( BalloonGirlRobotTest, 風船を殺すと下へ退く_スクリプトの_kill_も同じ )
{
	Fixture f;

	const auto id = f.create( "balloon" );
	f.factory.get_ref( id ).set_start_location( 3.f, 4.f, 5.f );
	f.factory.restart();

	f.factory.get_ref( id ).kill();

	EXPECT_TRUE( is_near( f.factory.get_ref( id ).get_location(), Vector( 0.f, -100.f, 0.f ) ) );
}

// ---- 少女 ----

TEST( BalloonGirlRobotTest, 少女は硬い物で_重力が無く_やり直すと回らず摩擦も無く立つ )
{
	Fixture f;

	const auto id = f.create( "girl" );
	auto* e = f.entity( id );

	ASSERT_NE( e->get_component< blue_sky::ecs::GirlComponent >(), nullptr );
	ASSERT_NE( e->get_component< blue_sky::ecs::FlickerComponent >(), nullptr );

	// 以前の Girl : 0.5 x 1.5 x 0.5・硬い・重力 0・質量 1。やり直すと回らない（回転のしやすさ 0）・摩擦 0
	const auto* traits = e->get_component< blue_sky::ecs::CollisionTraitsComponent >();
	EXPECT_TRUE( traits->is_hard );
	EXPECT_TRUE( traits->is_block );
	EXPECT_FALSE( traits->is_safe_footing );

	const auto* body = e->get_component< blue_sky::ecs::PhysicsBodyComponent >();
	EXPECT_FLOAT_EQ( body->height_offset, 0.75f );
	EXPECT_EQ( body->body->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE, 0 );

	e->get_component< blue_sky::ecs::GirlComponent >()->mode = blue_sky::ecs::GirlComponent::Mode::FLOAT;
	f.factory.get_ref( id ).set_flicker_scale( 10.f );
	f.factory.get_ref( id ).set_start_location( 1.f, 2.f, 3.f );

	f.factory.restart();

	EXPECT_TRUE( is_near( Vector( body->body->getGravity() ), Vector::Zero ) );
	EXPECT_TRUE( is_near( Vector( body->body->getAngularFactor() ), Vector::Zero ) );
	EXPECT_FLOAT_EQ( body->body->getFriction(), 0.f );

	EXPECT_EQ( e->get_component< blue_sky::ecs::GirlComponent >()->mode, blue_sky::ecs::GirlComponent::Mode::STAND );
	EXPECT_TRUE( is_near( e->get_component< blue_sky::ecs::GirlComponent >()->float_base_location, Vector( 1.f, 2.f, 3.f ) ) );
	EXPECT_FLOAT_EQ( e->get_component< blue_sky::ecs::FlickerComponent >()->scale, 1.f );
}

TEST( BalloonGirlRobotTest, 少女はアクション_float_で浮き_浮く中心へゆらぎの大きさで上下の速度を向ける )
{
	Fixture f;

	const auto id = f.create( "girl" );
	f.factory.get_ref( id ).set_start_location( 1.f, 2.f, 3.f );
	f.factory.restart();

	auto* system = f.manager.get_system< blue_sky::ecs::GirlSystem >();
	ASSERT_NE( system, nullptr );

	GameObjectRef girl = f.factory.get_ref( id );

	// 立っている間は動かさない
	girl.set_velocity( Vector( 1.f, 1.f, 1.f ) );
	system->update_girls( 2.f );
	EXPECT_TRUE( is_near( girl.get_velocity(), Vector( 1.f, 1.f, 1.f ) ) );

	// "float" で浮く（ステージの set_mode。以前の Girl::action）
	girl.action( "float" );
	EXPECT_EQ( f.entity( id )->get_component< blue_sky::ecs::GirlComponent >()->mode, blue_sky::ecs::GirlComponent::Mode::FLOAT );

	girl.set_flicker_scale( 10.f );
	system->update_girls( 2.f );

	// 以前の Girl::update : 速度を 0 にしてから update_velocity_by_flicker( 浮く中心, ゆらぎの大きさ )
	const float_t offset = blue_sky::ecs::flicker::height_offset( static_cast< unsigned int >( id ), 2.f, 10.f );
	EXPECT_TRUE( is_near( girl.get_velocity(), Vector( 0.f, 2.f + offset - girl.get_location().y(), 0.f ), 0.001f ) );

	// 知らないアクションはログを残すだけ（状態は変えない）
	girl.action( "rotation" );
	EXPECT_EQ( f.entity( id )->get_component< blue_sky::ecs::GirlComponent >()->mode, blue_sky::ecs::GirlComponent::Mode::FLOAT );
}

TEST( BalloonGirlRobotTest, 少女が目標地点に着くと止まり_そこが浮く中心になる )
{
	Fixture f;

	const auto id = f.create( "girl" );
	f.factory.restart();

	GameObjectRef girl = f.factory.get_ref( id );
	girl.set_velocity( Vector( 0.f, 5.f, 0.f ) );

	// 以前の Girl::on_arrive_at_target_location : 速度 0・浮く中心 = 今の位置
	blue_sky::ecs::EntityObject object( f.entity( id ) );
	object.stop_at_target_location( Vector( -5.f, 45.f, 25.f ) );
	object.notify_arrive_at_target_location();

	EXPECT_TRUE( is_near( girl.get_velocity(), Vector::Zero ) );
	EXPECT_TRUE( is_near( f.entity( id )->get_component< blue_sky::ecs::GirlComponent >()->float_base_location, Vector( -5.f, 45.f, 25.f ) ) );
}

TEST( BalloonGirlRobotTest, 少女への当たりは何も起こさない )
{
	Fixture f;

	const auto girl = f.create( "girl" );
	const auto stone = f.create( "stone" );
	f.factory.restart();

	const EntityId player = create_player( f );
	f.factory.restart();
	blue_sky::ecs::contact_rules::on_collide( * f.entity( player ), * f.entity( girl ) );
	blue_sky::ecs::contact_rules::on_collide( * f.entity( stone ), * f.entity( girl ) );

	EXPECT_EQ( player_of( f, player ).hp, 1 );
	EXPECT_EQ( player_of( f, player ).action_mode, blue_sky::ecs::PlayerBehaviorComponent::ActionMode::NONE );
	EXPECT_FALSE( blue_sky::ecs::EntityObject( f.entity( girl ) ).is_dead() );
	EXPECT_FALSE( blue_sky::ecs::EntityObject( f.entity( stone ) ).is_dead() );
}

// ---- 敵 ----

namespace
{

using RobotMode = blue_sky::ecs::RobotBehaviorComponent::Mode;
using RobotLook = blue_sky::ecs::RobotBehaviorComponent::Look;

/// 原点で前（+Z）を向いた敵と、追う相手（プレイヤー。剛体なし = 速度 0。位置は部品から読まれる）
struct RobotFixture : public Fixture
{
	EntityId robot = 0;
	EntityId target_id = 0;

	blue_sky::ecs::RobotBehaviorSystem* system = nullptr;

	RobotFixture()
	{
		robot = create( "robot" );
		factory.restart();

		target_id = factory.create_player();
		set_target_location( Vector( 0.f, 0.f, 5.f ) );

		system = manager.get_system< blue_sky::ecs::RobotBehaviorSystem >();
		system->set_target( robot, target_id );
	}

	void set_target_location( const Vector& location ) { factory.get_ref( target_id ).set_location( location ); }
	blue_sky::ecs::PlayerBehaviorComponent& target() { return * entity( target_id )->get_component< blue_sky::ecs::PlayerBehaviorComponent >(); }

	blue_sky::ecs::RobotBehaviorComponent* component() { return entity( robot )->get_component< blue_sky::ecs::RobotBehaviorComponent >(); }
	RobotMode mode() { return component()->mode; }

	/// dt 秒ずつ count 回動かす
	void update( int count, float_t dt = 0.1f, float_t elapsed = 1.f )
	{
		for ( int i = 0; i < count; i++ )
		{
			system->update_robot( robot, dt, elapsed );
		}
	}
};

} // namespace

TEST( BalloonGirlRobotTest, 敵は硬い物で_やり直すと質量_50_回らず摩擦も無く立ち_始めの位置と_5_m_前を巡回の地点にする )
{
	Fixture f;

	const auto id = f.create( "robot" );
	auto* e = f.entity( id );

	ASSERT_NE( e->get_component< blue_sky::ecs::RobotBehaviorComponent >(), nullptr );
	ASSERT_NE( e->get_component< blue_sky::ecs::PatrolComponent >(), nullptr );
	ASSERT_NE( e->get_component< blue_sky::ecs::FlickerComponent >(), nullptr );

	// 以前の Robot : 1 x 2 x 0.5・硬い・地球の重力・作ったときの質量 1
	const auto* traits = e->get_component< blue_sky::ecs::CollisionTraitsComponent >();
	EXPECT_TRUE( traits->is_hard );
	EXPECT_TRUE( traits->is_block );

	const auto* body = e->get_component< blue_sky::ecs::PhysicsBodyComponent >();
	EXPECT_FLOAT_EQ( body->height_offset, 1.f );
	EXPECT_FLOAT_EQ( body->body->getInvMass(), 1.f );

	GameObjectRef robot = f.factory.get_ref( id );
	robot.set_start_location( 1.f, 2.f, 3.f );
	robot.set_start_direction_degree( 90.f );
	robot.action( "rotation" );

	f.factory.restart();

	EXPECT_FLOAT_EQ( body->body->getInvMass(), 1.f / 50.f );
	EXPECT_TRUE( is_near( Vector( body->body->getAngularFactor() ), Vector::Zero ) );
	EXPECT_FLOAT_EQ( body->body->getFriction(), 0.f );
	EXPECT_TRUE( is_near( Vector( body->body->getGravity() ), Vector( 0.f, -9.8f, 0.f ) ) );

	const auto* robot_component = e->get_component< blue_sky::ecs::RobotBehaviorComponent >();
	EXPECT_EQ( robot_component->mode, RobotMode::STAND );
	EXPECT_EQ( robot_component->look, RobotLook::NORMAL );

	// 巡回の地点（以前の Robot::restart）: 始めの位置と、向き 90 度の前（+X）へ 5 m
	const auto* patrol = e->get_component< blue_sky::ecs::PatrolComponent >();
	ASSERT_EQ( patrol->waypoints.size(), 2u );
	EXPECT_TRUE( is_near( patrol->waypoints[ 0 ], Vector( 1.f, 2.f, 3.f ) ) );
	EXPECT_TRUE( is_near( patrol->waypoints[ 1 ], Vector( 6.f, 2.f, 3.f ), 0.001f ) );
	EXPECT_EQ( patrol->current, 0u );
	EXPECT_FLOAT_EQ( patrol->move_speed, 0.1f );
}

TEST( BalloonGirlRobotTest, 敵の視野は距離_3_10_20_m_で広さが変わり_20_m_より遠くは見えない )
{
	RobotFixture f;

	auto visible_at = [ & ] ( const Vector& location )
	{
		f.set_target_location( location );
		return f.system->is_target_visible( f.robot );
	};

	// 3 m 以内 : 真横は見え、真後ろは見えない（視野角 -0.25）
	EXPECT_TRUE( visible_at( Vector( 2.f, 0.f, 0.f ) ) );
	EXPECT_FALSE( visible_at( Vector( 0.f, 0.f, -2.f ) ) );

	// 3〜10 m : 正面は見え、真横は見えない（0.25）
	EXPECT_TRUE( visible_at( Vector( 0.f, 0.f, 5.f ) ) );
	EXPECT_FALSE( visible_at( Vector( 5.f, 0.f, 0.f ) ) );

	// 10〜20 m : 正面から 45 度は見えない（0.75 > cos 45°）
	EXPECT_TRUE( visible_at( Vector( 0.f, 0.f, 15.f ) ) );
	EXPECT_FALSE( visible_at( Vector( 10.f, 0.f, 10.f ) ) );

	// 20 m より遠く・相手がいない
	EXPECT_FALSE( visible_at( Vector( 0.f, 0.f, 25.f ) ) );

	f.manager.destroy_entity( f.target_id );
	EXPECT_FALSE( visible_at( Vector( 0.f, 0.f, 5.f ) ) );
}

TEST( BalloonGirlRobotTest, 立つ敵は相手が見えると注目し_3_秒後も見えていれば追い_相手の方へ秒速_3_で向かう )
{
	RobotFixture f;

	// 見えない所にいる間は立ったまま（速度 0）
	f.set_target_location( Vector( 0.f, 0.f, -10.f ) );
	f.update( 1 );
	EXPECT_EQ( f.mode(), RobotMode::STAND );

	// 見えると注目（警戒の見た目）
	f.set_target_location( Vector( 0.f, 0.f, 5.f ) );
	f.update( 1 );
	EXPECT_EQ( f.mode(), RobotMode::ATTENTION );
	EXPECT_EQ( f.component()->look, RobotLook::WARNING );
	EXPECT_EQ( f.component()->mode_backup, RobotMode::STAND );

	// 3 秒たつまでは注目のまま
	f.update( 29 );
	EXPECT_EQ( f.mode(), RobotMode::ATTENTION );

	f.update( 2 );
	EXPECT_EQ( f.mode(), RobotMode::CHASE );

	// 追う : 相手の方（+Z）へ秒速 3
	f.update( 1 );
	const Vector v = f.factory.get_ref( f.robot ).get_velocity();
	EXPECT_NEAR( v.x(), 0.f, 0.001f );
	EXPECT_NEAR( v.z(), 3.f, 0.001f );
}

TEST( BalloonGirlRobotTest, 注目の後に見えなくなっていれば元の状態へ戻り_追って_10_秒たって見えなければ回る )
{
	RobotFixture f;

	f.factory.get_ref( f.robot ).action( "rotation" );

	f.set_target_location( Vector( 0.f, 0.f, 5.f ) );
	f.update( 1 );
	ASSERT_EQ( f.mode(), RobotMode::ATTENTION );
	EXPECT_EQ( f.component()->mode_backup, RobotMode::ROTATION );

	// 見えない所へ（20 m より遠く）: 3 秒後に元の 回る へ（ふだんの見た目）
	f.set_target_location( Vector( 0.f, 0.f, -30.f ) );
	f.update( 31 );
	EXPECT_EQ( f.mode(), RobotMode::ROTATION );
	EXPECT_EQ( f.component()->look, RobotLook::NORMAL );

	// 追っている間は 10 秒たつまで追い続け、見えなければ 回る へ
	f.component()->mode = RobotMode::CHASE;
	f.component()->timer = 0.f;

	f.update( 99 );
	EXPECT_EQ( f.mode(), RobotMode::CHASE );

	f.update( 2 );
	EXPECT_EQ( f.mode(), RobotMode::ROTATION );
	EXPECT_EQ( f.component()->look, RobotLook::NORMAL );
}

TEST( BalloonGirlRobotTest, 回る敵は_1_フレームに_0_25_度回り_追う間の見た目は_1_秒に_2_回点滅する )
{
	RobotFixture f;

	f.set_target_location( Vector( 0.f, 0.f, -30.f ) );
	f.factory.get_ref( f.robot ).action( "rotation" );
	f.update( 4 );

	EXPECT_FLOAT_EQ( f.factory.get_ref( f.robot ).get_direction_degree(), 1.f );

	// 追う間 : 0.25 秒ごとに警報と警戒（以前の is_visible_in_blink( 2 )）
	f.component()->mode = RobotMode::CHASE;

	f.system->update_robot( f.robot, 0.01f, 1.1f );
	EXPECT_EQ( f.component()->look, RobotLook::ALARM );

	f.system->update_robot( f.robot, 0.01f, 1.3f );
	EXPECT_EQ( f.component()->look, RobotLook::WARNING );
}

TEST( BalloonGirlRobotTest, 立つ_回る敵は相手に当たると止まり_追う敵に当たった相手は傷つく )
{
	RobotFixture f;

	f.target().hp = 2;

	// 追っている敵 : 相手は傷つき（HP が減り、押し返されて少しの間 操作できない）、敵は止まらない
	f.component()->mode = RobotMode::CHASE;
	blue_sky::ecs::contact_rules::on_collide( * f.entity( f.target_id ), * f.entity( f.robot ) );

	EXPECT_EQ( f.target().hp, 1 );
	EXPECT_TRUE( f.target().is_uncontrollable() );
	EXPECT_EQ( f.mode(), RobotMode::CHASE );

	// 注目している敵 : 止まらない。相手も傷つかない
	f.component()->mode = RobotMode::ATTENTION;
	blue_sky::ecs::contact_rules::on_collide( * f.entity( f.robot ), * f.entity( f.target_id ) );
	EXPECT_EQ( f.mode(), RobotMode::ATTENTION );
	EXPECT_EQ( f.target().hp, 1 );

	// 回っている敵 : 止まる。止まった敵の速度は毎フレーム半分になる
	f.component()->mode = RobotMode::ROTATION;
	blue_sky::ecs::contact_rules::on_collide( * f.entity( f.target_id ), * f.entity( f.robot ) );

	EXPECT_EQ( f.mode(), RobotMode::SHUTDOWN );
	EXPECT_EQ( f.target().hp, 1 );

	f.factory.get_ref( f.robot ).set_velocity( Vector( 4.f, 0.f, 0.f ) );
	f.update( 1 );
	EXPECT_NEAR( f.factory.get_ref( f.robot ).get_velocity().x(), 2.f, 0.001f );
	EXPECT_EQ( f.mode(), RobotMode::SHUTDOWN );
}

TEST( BalloonGirlRobotTest, 立つ敵はこちらへ飛んでくる石に当たると止まり_離れていく石では止まらない )
{
	RobotFixture f;

	const auto stone = f.create( "stone" );
	f.factory.restart();
	f.set_target_location( Vector( 0.f, 0.f, -30.f ) );

	GameObjectRef s = f.factory.get_ref( stone );
	s.set_location( Vector( 0.f, 1.f, 2.f ) );

	// 離れていく（+Z へ飛ぶ）石 : 止まらない。石は拾われない
	s.set_velocity( Vector( 0.f, 0.f, 8.f ) );
	blue_sky::ecs::contact_rules::on_collide( * f.entity( stone ), * f.entity( f.robot ) );

	EXPECT_EQ( f.mode(), RobotMode::STAND );
	EXPECT_FALSE( blue_sky::ecs::EntityObject( f.entity( stone ) ).is_dead() );

	// こちらへ飛んでくる石 : 止まる（組の向きによらない）
	s.set_velocity( Vector( 0.f, 0.f, -8.f ) );
	blue_sky::ecs::contact_rules::on_collide( * f.entity( f.robot ), * f.entity( stone ) );

	EXPECT_EQ( f.mode(), RobotMode::SHUTDOWN );
}

TEST( BalloonGirlRobotTest, 浮く敵は始めの位置でゆらぎ_やさしい見た目になり_相手の方を向く )
{
	RobotFixture f;

	GameObjectRef robot = f.factory.get_ref( f.robot );
	robot.action( "float" );
	robot.set_flicker_scale( 10.f );

	f.set_target_location( Vector( 10.f, 0.f, 0.f ) );
	f.system->update_robot( f.robot, 0.1f, 2.f );

	EXPECT_EQ( f.mode(), RobotMode::FLOAT );
	EXPECT_EQ( f.component()->look, RobotLook::TENDER );

	// 以前の Robot::update（浮く）: update_velocity_by_flicker( 始めの位置, ゆらぎの大きさ )・chase_direction_to( 相手, 0.5 )
	const float_t offset = blue_sky::ecs::flicker::height_offset( static_cast< unsigned int >( f.robot ), 2.f, 10.f );
	EXPECT_NEAR( robot.get_velocity().y(), offset, 0.001f );
	EXPECT_FLOAT_EQ( robot.get_direction_degree(), 0.5f );

	// 知らないアクションは状態を変えない
	robot.action( "unknown" );
	EXPECT_EQ( f.mode(), RobotMode::FLOAT );
}
