#include "pch.h"

#include <blue_sky/ObjectFactory.h>
#include <blue_sky/GameObjectRef.h>
#include <blue_sky/ActiveObjectManager.h>
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

bool is_near( const Vector& a, const Vector& b, float_t e = 0.0001f )
{
	return std::abs( a.x() - b.x() ) < e && std::abs( a.y() - b.y() ) < e && std::abs( a.z() - b.z() ) < e;
}

/// 当たりを受け取る偽の物体（ActiveObject の代わり）
class FakeObject : public blue_sky::GameObject
{
public:
	void restart() override { }
	void update() override { }
	float_t get_collision_width() const override { return 1.f; }
	float_t get_collision_height() const override { return 1.f; }
	float_t get_collision_depth() const override { return 1.f; }
};

/// 風船をつかむ偽のプレイヤー（grabs が false ならつかまない = 揺らぎ中のプレイヤー・ほかの物体）
class FakeHolder : public FakeObject, public blue_sky::ecs::BalloonHolder
{
public:
	bool grabs = true;
	int touch_count = 0;

	std::optional< EntityId > balloon;
	bool in_action = true;
	bool pre_finish = false;
	Vector hand = Vector( 1.f, 2.f, 3.f );

	blue_sky::ecs::BalloonHolder* on_collide_with_balloon( EntityId id ) override
	{
		touch_count++;

		if ( ! grabs )
		{
			return nullptr;
		}

		balloon = id;
		return this;
	}

	bool is_holding_balloon( EntityId id ) const override { return balloon == id; }
	bool is_in_balloon_action() const override { return in_action; }
	bool is_balloon_action_pre_finish() const override { return pre_finish; }
	Vector get_balloon_hold_location() const override { return hand; }
};

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

	const EntityId holder_id = f.manager.create_entity()->get_id();

	// つかまない物体（揺らぎ中のプレイヤー）: 知らせるが、つかまれない
	FakeHolder other;
	other.grabs = false;
	blue_sky::ActiveObjectManager::on_collide_with_entity_object( other, 100, * f.entity( id ) );

	EXPECT_EQ( other.touch_count, 1 );
	EXPECT_FALSE( f.entity( id )->get_component< blue_sky::ecs::BalloonHoldComponent >()->is_held() );

	// プレイヤー : つかむ
	FakeHolder player;
	blue_sky::ActiveObjectManager::on_collide_with_entity_object( player, holder_id, * f.entity( id ) );

	EXPECT_EQ( player.touch_count, 1 );
	EXPECT_TRUE( f.entity( id )->get_component< blue_sky::ecs::BalloonHoldComponent >()->is_held() );

	// つかまれた風船は、もう知らせない（以前は Player::on_collide_with( Balloon* ) が balloon->get_player() を見て戻っていた）
	blue_sky::ActiveObjectManager::on_collide_with_entity_object( player, holder_id, * f.entity( id ) );
	EXPECT_EQ( player.touch_count, 1 );

	// 手元に置かれ、向きは 0
	f.factory.get_ref( id ).set_direction_degree( 30.f );
	f.manager.get_system< blue_sky::ecs::BalloonHoldSystem >()->update_balloon( id, 1.f );

	EXPECT_TRUE( is_near( f.factory.get_ref( id ).get_location(), player.hand ) );
	EXPECT_FLOAT_EQ( f.factory.get_ref( id ).get_direction_degree(), 0.f );
	EXPECT_FALSE( blue_sky::ecs::EntityObject( f.entity( id ) ).is_dead() );
}

TEST( BalloonGirlRobotTest, つかんだ物体が別の風船を持つか_風船の動作が終わると_風船は割れて下へ退く )
{
	Fixture f;

	const auto first = f.create( "balloon" );
	const auto second = f.create( "balloon" );
	f.factory.restart();

	const EntityId holder_id = f.manager.create_entity()->get_id();
	auto* system = f.manager.get_system< blue_sky::ecs::BalloonHoldSystem >();

	FakeHolder player;
	blue_sky::ActiveObjectManager::on_collide_with_entity_object( player, holder_id, * f.entity( first ) );
	blue_sky::ActiveObjectManager::on_collide_with_entity_object( player, holder_id, * f.entity( second ) );

	system->update_balloons( 1.f );

	// 前の風船は割れて (0, -100, 0) へ（以前の Balloon::kill）。今の風船は手元
	EXPECT_TRUE( blue_sky::ecs::EntityObject( f.entity( first ) ).is_dead() );
	EXPECT_TRUE( is_near( f.factory.get_ref( first ).get_location(), Vector( 0.f, -100.f, 0.f ) ) );
	EXPECT_FALSE( blue_sky::ecs::EntityObject( f.entity( second ) ).is_dead() );

	// 動作が終わると、今の風船も割れる
	player.in_action = false;
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

	const EntityId holder_id = f.manager.create_entity()->get_id();
	auto* system = f.manager.get_system< blue_sky::ecs::BalloonHoldSystem >();
	const auto* render = f.entity( id )->get_component< blue_sky::ecs::ObjectRenderComponent >();

	FakeHolder player;
	blue_sky::ActiveObjectManager::on_collide_with_entity_object( player, holder_id, * f.entity( id ) );

	// 終わりかけでなければ隠さない
	system->update_balloon( id, 0.1f );
	EXPECT_FALSE( render->is_hidden );

	// 終わりかけ : 1/6 秒ごとの前半は見え、後半は隠れる（以前の is_visible_in_blink( 6 )）
	player.pre_finish = true;

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

	const EntityId holder_id = f.manager.create_entity()->get_id();

	FakeHolder player;
	blue_sky::ActiveObjectManager::on_collide_with_entity_object( player, holder_id, * f.entity( id ) );

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
	blue_sky::ActiveObjectManager::on_collide_entity_objects( * f.entity( stone ), * f.entity( balloon ) );

	EXPECT_TRUE( blue_sky::ecs::EntityObject( f.entity( balloon ) ).is_dead() );
	EXPECT_TRUE( is_near( f.factory.get_ref( balloon ).get_location(), Vector( 0.f, -100.f, 0.f ) ) );
	EXPECT_FALSE( blue_sky::ecs::EntityObject( f.entity( stone ) ).is_dead() );

	f.factory.restart();
	blue_sky::ActiveObjectManager::on_collide_entity_objects( * f.entity( balloon ), * f.entity( stone ) );

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
