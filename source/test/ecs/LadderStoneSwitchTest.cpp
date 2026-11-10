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

#include <core/ecs/EntityManager.h>

#include <btBulletDynamicsCommon.h>

#include <cmath>

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
