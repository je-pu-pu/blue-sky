#include "pch.h"

#include <blue_sky/ObjectFactory.h>
#include <blue_sky/GameObjectRef.h>
#include <blue_sky/ActiveObjectManager.h>
#include <blue_sky/ActiveObjectPhysics.h>

#include <blue_sky/ecs/EntityObject.h>
#include <blue_sky/ecs/component/ActiveObjectLinkComponent.h>
#include <blue_sky/ecs/component/CollisionTraitsComponent.h>
#include <blue_sky/ecs/component/ObjectRenderComponent.h>
#include <blue_sky/ecs/component/PhysicsBodyComponent.h>
#include <blue_sky/ecs/component/SimpleObjectComponent.h>
#include <blue_sky/ecs/component/StaticObjectComponent.h>
#include <blue_sky/ecs/component/TargetMoverComponent.h>
#include <blue_sky/ecs/component/TargetLocationComponent.h>
#include <blue_sky/ecs/system/MoveToTargetSystem.h>
#include <blue_sky/ecs/system/TranslationSystem.h>
#include <blue_sky/ecs/system/SpinSystem.h>
#include <blue_sky/ecs/component/PickupComponent.h>

#include <core/ecs/EntityManager.h>
#include <core/ecs/component/TransformComponent.h>
#include <core/physics/EntityTag.h>

#include <common/math.h>

#include <btBulletDynamicsCommon.h>

#include <cmath>
#include <sstream>

/**
 * 段階 4 : 物体を種類の名前から作る ObjectFactory と、ECS だけの物体（動かない物・動く床）のテスト
 *
 * 描画（GraphicsManager）は無しで作る（モデル・定数バッファは用意しない）。物理の世界は本物（ActiveObjectPhysics）を使う
 */

using blue_sky::ObjectFactory;
using blue_sky::GameObjectRef;
using core::ecs::EntityManager;
using core::Vector;

namespace
{

/// 描画なし・ActiveObject なしで工場を作る
struct FactoryFixture
{
	blue_sky::ActiveObjectPhysics physics;
	EntityManager manager;
	ObjectFactory factory{ & manager, nullptr, & physics, nullptr };

	~FactoryFixture()
	{
		// 剛体の持ち主（部品）を、物理の世界より先に消す
		factory.clear();
		manager.clear();
	}

	core::ecs::Entity* entity( core::ecs::EntityId id ) { return manager.find( id ); }

	ObjectFactory::EntityId create_from_line( const char* line )
	{
		std::stringstream ss;
		ss << line;
		return factory.create_static_object( ss );
	}
};

/// 当たりを受け取る偽の物体（ActiveObject の代わり。GameObject の仮想関数が呼ばれた回数を数える）
class FakeGameObject : public blue_sky::GameObject
{
public:
	int static_object_count = 0;

	void restart() override { }
	void update() override { }
	float_t get_collision_width() const override { return 1.f; }
	float_t get_collision_height() const override { return 1.f; }
	float_t get_collision_depth() const override { return 1.f; }

	void on_collide_with_static_object() override { static_object_count++; }
};

/// 偽の物体の Entity の番号（当たりの互換の層に渡す。梯子・スイッチが触れた物体を覚えるのに使う）
constexpr core::ecs::EntityId FakeObjectId = 1000;

bool is_near( const Vector& a, const Vector& b, float_t e = 0.0001f )
{
	return std::abs( a.x() - b.x() ) < e && std::abs( a.y() - b.y() ) < e && std::abs( a.z() - b.z() ) < e;
}

} // namespace

// ---- 作った Entity が持つ部品 ----

TEST( ObjectFactoryTest, 動かない物は_ActiveObject_を持たず部品だけで作られる )
{
	FactoryFixture f;

	const auto id = f.create_from_line( "wall-3 1 2 3 90 0 0" );
	auto* e = f.entity( id );

	ASSERT_NE( e, nullptr );
	EXPECT_NE( e->get_component< core::ecs::TransformComponent >(), nullptr );
	EXPECT_NE( e->get_component< blue_sky::ecs::PhysicsBodyComponent >(), nullptr );
	EXPECT_NE( e->get_component< blue_sky::ecs::CollisionTraitsComponent >(), nullptr );
	EXPECT_NE( e->get_component< blue_sky::ecs::StaticObjectComponent >(), nullptr );
	EXPECT_NE( e->get_component< blue_sky::ecs::SimpleObjectComponent >(), nullptr );
	EXPECT_NE( e->get_component< blue_sky::ecs::ObjectRenderComponent >(), nullptr );
	EXPECT_EQ( e->get_component< blue_sky::ecs::TranslationComponent >(), nullptr );

	// ActiveObject を持たない（互換の層の印が無い・目標へ動かす窓口は部品を直接動かす印の nullptr）
	EXPECT_EQ( e->get_component< blue_sky::ecs::ActiveObjectLinkComponent >(), nullptr );
	ASSERT_NE( e->get_component< blue_sky::ecs::TargetMoverComponent >(), nullptr );
	EXPECT_EQ( e->get_component< blue_sky::ecs::TargetMoverComponent >()->mover, nullptr );

	EXPECT_EQ( f.factory.get_entity_object_list().size(), 1u );
	EXPECT_EQ( f.factory.get_ref( id ).get_active_object(), nullptr );
}

TEST( ObjectFactoryTest, 動かない物の性質は以前の_StaticObject_と同じ_硬く壁になり足場では死ぬ )
{
	FactoryFixture f;

	const auto* traits = f.entity( f.create_from_line( "wall-3 0 0 0" ) )->get_component< blue_sky::ecs::CollisionTraitsComponent >();

	EXPECT_FALSE( traits->is_ghost );
	EXPECT_TRUE( traits->is_hard );
	EXPECT_TRUE( traits->is_block );
	EXPECT_FALSE( traits->is_safe_footing );
	EXPECT_FALSE( traits->is_dead );
}

TEST( ObjectFactoryTest, 種類ごとの大きさ_質量_向き_形はステージの行から決まる )
{
	FactoryFixture f;

	// wall-3 : 4 x 2.5 x 0.2・質量 100。4 つ目の数（rx）が向きになる（以前と同じ）
	const auto wall = f.create_from_line( "wall-3 1 2 3 90 0 0" );
	const auto* wall_state = f.entity( wall )->get_component< blue_sky::ecs::SimpleObjectComponent >();
	const auto* wall_body = f.entity( wall )->get_component< blue_sky::ecs::PhysicsBodyComponent >();

	EXPECT_EQ( wall_state->kind, "wall-3" );
	EXPECT_TRUE( is_near( wall_state->start_location, Vector( 1.f, 2.f, 3.f ) ) );
	EXPECT_FLOAT_EQ( wall_state->start_direction_degree, 90.f );
	EXPECT_FLOAT_EQ( wall_body->height_offset, 1.25f );
	EXPECT_FLOAT_EQ( wall_body->body->getInvMass(), 1.f / 100.f );
	EXPECT_EQ( wall_body->body->getCollisionShape()->getShapeType(), BOX_SHAPE_PROXYTYPE );

	// soda-can-1 だけ円柱
	const auto can = f.create_from_line( "soda-can-1 0 0 0" );
	EXPECT_EQ( f.entity( can )->get_component< blue_sky::ecs::PhysicsBodyComponent >()->body->getCollisionShape()->getShapeType(), CYLINDER_SHAPE_PROXYTYPE );

	// box-2x2x2 は摩擦 10
	const auto box = f.create_from_line( "box-2x2x2 0 0 0" );
	EXPECT_FLOAT_EQ( f.entity( box )->get_component< blue_sky::ecs::PhysicsBodyComponent >()->body->getFriction(), 10.f );

	// 知らない種類は大きさ 0・質量 0（動かない）
	const auto tree = f.create_from_line( "tree-1 0 0 0" );
	EXPECT_FLOAT_EQ( f.entity( tree )->get_component< blue_sky::ecs::PhysicsBodyComponent >()->body->getInvMass(), 0.f );
}

TEST( ObjectFactoryTest, 剛体には_Entity_の番号のタグが付き_足元から高さの半分上に置かれる )
{
	FactoryFixture f;

	const auto id = f.create_from_line( "wall-3 1 2 3" );
	const auto* body = f.entity( id )->get_component< blue_sky::ecs::PhysicsBodyComponent >()->body;

	ASSERT_TRUE( core::physics::get_entity_tag( body ).has_value() );
	EXPECT_EQ( * core::physics::get_entity_tag( body ), id );

	const btVector3& origin = body->getWorldTransform().getOrigin();
	EXPECT_NEAR( origin.x(), 1.f, 0.0001f );
	EXPECT_NEAR( origin.y(), 2.f + 1.25f, 0.0001f );
	EXPECT_NEAR( origin.z(), 3.f, 0.0001f );
}

TEST( ObjectFactoryTest, 種類の名前から作る_static_は_ECS_だけの物体_知らない種類は作らない )
{
	FactoryFixture f;

	const auto id = f.factory.create_object( "static" );
	ASSERT_TRUE( id.has_value() );
	EXPECT_NE( f.entity( * id )->get_component< blue_sky::ecs::StaticObjectComponent >(), nullptr );

	// まだ ActiveObject の種類は ActiveObjectManager が作る（このテストでは無いので作れない）
	EXPECT_FALSE( f.factory.create_object( "robot" ).has_value() );
	EXPECT_FALSE( f.factory.create_object( "no-such-kind" ).has_value() );
}

TEST( ObjectFactoryTest, 描くだけの物は剛体を持たない )
{
	FactoryFixture f;

	const auto id = f.factory.create_model_object( nullptr );

	EXPECT_EQ( f.entity( id )->get_component< blue_sky::ecs::PhysicsBodyComponent >(), nullptr );
	EXPECT_NE( f.entity( id )->get_component< blue_sky::ecs::ObjectRenderComponent >(), nullptr );
}

// ---- 名前・片付け ----

TEST( ObjectFactoryTest, 名前で引け_片付けると引けなくなり_剛体も世界から消える )
{
	FactoryFixture f;

	const int before = f.physics.get_rigid_body_count();

	const auto id = f.create_from_line( "wall-3 0 0 0" );
	f.factory.name( "wall-1", id );

	ASSERT_TRUE( f.factory.find( "wall-1" ).has_value() );
	EXPECT_EQ( * f.factory.find( "wall-1" ), id );
	EXPECT_FALSE( f.factory.find( "wall-2" ).has_value() );
	EXPECT_EQ( f.physics.get_rigid_body_count(), before + 1 );

	f.factory.clear();

	EXPECT_FALSE( f.factory.find( "wall-1" ).has_value() );
	EXPECT_FALSE( f.manager.is_alive( id ) );
	EXPECT_EQ( f.physics.get_rigid_body_count(), before );
	EXPECT_TRUE( f.factory.get_entity_object_list().empty() );
}

TEST( ObjectFactoryTest, 消えた物体の名前は引けない )
{
	FactoryFixture f;

	const auto id = f.create_from_line( "wall-3 0 0 0" );
	f.factory.name( "wall", id );
	f.manager.destroy_entity( id );

	EXPECT_FALSE( f.factory.find( "wall" ).has_value() );
	EXPECT_FALSE( f.factory.find_ref( "wall" ).has_value() );
}

// ---- やり直し・死亡・表示（以前の ActiveObject::restart・kill・action と同じ） ----

TEST( ObjectFactoryTest, やり直すと始めの位置と向きに戻り_剛体も止まる )
{
	FactoryFixture f;

	const auto id = f.create_from_line( "wall-3 1 2 3 90 0 0" );
	GameObjectRef o = f.factory.get_ref( id );

	f.factory.restart();

	EXPECT_TRUE( is_near( o.get_location(), Vector( 1.f, 2.f, 3.f ) ) );

	// 動かしてからやり直す
	o.set_location( Vector( 10.f, 20.f, 30.f ) );
	o.set_velocity( Vector( 1.f, 0.f, 0.f ) );
	f.factory.restart();

	EXPECT_TRUE( is_near( o.get_location(), Vector( 1.f, 2.f, 3.f ) ) );

	const auto* body = f.entity( id )->get_component< blue_sky::ecs::PhysicsBodyComponent >()->body;
	EXPECT_NEAR( body->getLinearVelocity().length(), 0.f, 0.0001f );

	// 向き（Y 軸 90 度）が剛体にも送られている
	const btQuaternion q = body->getWorldTransform().getRotation();
	EXPECT_NEAR( std::abs( q.getAngle() ), math::degree_to_radian( 90.f ), 0.001f );
	EXPECT_NEAR( body->getWorldTransform().getOrigin().y(), 2.f + 1.25f, 0.0001f );
}

TEST( ObjectFactoryTest, 死なせると当たりを起こさず描かれず_やり直すと戻る )
{
	FactoryFixture f;

	const auto id = f.create_from_line( "wall-3 0 0 0" );
	GameObjectRef o = f.factory.get_ref( id );

	o.kill();

	const auto* traits = f.entity( id )->get_component< blue_sky::ecs::CollisionTraitsComponent >();
	const auto* render = f.entity( id )->get_component< blue_sky::ecs::ObjectRenderComponent >();

	EXPECT_TRUE( traits->is_dead );
	EXPECT_FALSE( render->is_visible );
	EXPECT_FALSE( render->is_mesh_visible );
	EXPECT_FALSE( render->is_line_visible );

	o.restart();

	EXPECT_FALSE( traits->is_dead );
	EXPECT_TRUE( render->is_visible );

	// アクション set_visible メッシュ 線
	o.action( "set_visible 0 1" );

	EXPECT_FALSE( render->is_mesh_visible );
	EXPECT_TRUE( render->is_line_visible );
	EXPECT_TRUE( render->is_visible );
}

// ---- 目標地点（スイッチで動く壁。ActiveObject を持たないので部品を直接動かす） ----

TEST( ObjectFactoryTest, ECS_だけの物体も目標地点へ動き_着いたら止まる )
{
	FactoryFixture f;

	const auto id = f.create_from_line( "wall-3 0 1 0" );
	f.factory.restart();

	GameObjectRef o = f.factory.get_ref( id );
	o.set_target_location( Vector( 0.f, 4.f, 0.f ), 1.f );

	auto* system = f.manager.get_system< blue_sky::ecs::MoveToTargetSystem >();
	ASSERT_NE( system, nullptr );
	EXPECT_EQ( system->debug_target_count(), 1u );

	const auto* body = f.entity( id )->get_component< blue_sky::ecs::PhysicsBodyComponent >()->body;

	// まだ遠い : 目標へ向けて速さ 1 の速度を設定する
	system->update_targets( 1.f / 60.f );
	EXPECT_NEAR( body->getLinearVelocity().y(), 1.f, 0.0001f );
	EXPECT_NE( f.entity( id )->get_component< blue_sky::ecs::TargetLocationComponent >(), nullptr );

	// 目標の近くに置くと、次のフレームで目標に置いて止め、目標を外す
	o.set_location( Vector( 0.f, 3.95f, 0.f ) );
	system->update_targets( 1.f / 60.f );

	EXPECT_TRUE( is_near( o.get_location(), Vector( 0.f, 4.f, 0.f ) ) );
	EXPECT_NEAR( body->getLinearVelocity().length(), 0.f, 0.0001f );
	EXPECT_EQ( f.entity( id )->get_component< blue_sky::ecs::TargetLocationComponent >(), nullptr );
	EXPECT_EQ( system->debug_target_count(), 0u );
}

TEST( ObjectFactoryTest, ECS_だけの物体も目標の向きを追う )
{
	FactoryFixture f;

	const auto id = f.create_from_line( "wall-3 0 0 0" );
	f.factory.restart();

	GameObjectRef o = f.factory.get_ref( id );
	o.set_target_direction( 90.f, 30.f );

	auto* system = f.manager.get_system< blue_sky::ecs::MoveToTargetSystem >();
	system->update_targets( 1.f / 60.f );

	// 1 フレームで 30 度だけ近づく（以前の ActiveObject::chase_direction_degree と同じ）
	EXPECT_FLOAT_EQ( f.entity( id )->get_component< blue_sky::ecs::SimpleObjectComponent >()->direction_degree, 30.f );
}

// ---- 動く床（以前の TranslationObject） ----

TEST( ObjectFactoryTest, 動く床は始めの位置を中心に_sin_cos_で動く )
{
	FactoryFixture f;

	const auto id = f.factory.create_translation_object( Vector( 1.f, 2.f, 3.f ), 2.f, 3.f, 4.f, 1.f );
	auto* e = f.entity( id );

	ASSERT_NE( e->get_component< blue_sky::ecs::TranslationComponent >(), nullptr );
	EXPECT_NE( e->get_component< blue_sky::ecs::StaticObjectComponent >(), nullptr );	// 以前も StaticObject の派生だった

	auto* system = f.manager.get_system< blue_sky::ecs::TranslationSystem >();
	ASSERT_NE( system, nullptr );

	system->update_translations( 0.5f );

	const Vector expected = Vector( 1.f, 2.f, 3.f ) + Vector( std::cos( 0.5f ) * 2.f, std::sin( 0.5f ) * 3.f, std::cos( 0.5f ) * 4.f );
	EXPECT_TRUE( is_near( f.factory.get_ref( id ).get_location(), expected ) );
}

// ---- 当たり（ActiveObject と ECS だけの物体の組。部品の組み合わせで決める） ----

TEST( ObjectFactoryTest, 動かない物に当たると相手に一度だけ知らせる_印の無い物には何もしない )
{
	FactoryFixture f;
	FakeGameObject player;

	const auto wall = f.create_from_line( "wall-3 0 0 0" );
	blue_sky::ActiveObjectManager::on_collide_with_entity_object( player, FakeObjectId, * f.entity( wall ) );

	EXPECT_EQ( player.static_object_count, 1 );

	// 動かない物の印が無い Entity
	auto* other = f.manager.create_entity();
	blue_sky::ActiveObjectManager::on_collide_with_entity_object( player, FakeObjectId, * other );

	EXPECT_EQ( player.static_object_count, 1 );
}

// ---- 拾う物（段階 4 ②） ----

namespace
{

/// 拾う物を拾う偽のプレイヤー（拾った種類を記録する）。picks が false なら拾わない物体（敵など）
class FakePicker : public FakeGameObject
{
public:
	bool picks = true;
	std::vector< blue_sky::ecs::PickupKind > picked;

	bool on_pick_up( blue_sky::ecs::PickupKind kind ) override
	{
		if ( picks )
		{
			picked.push_back( kind );
		}

		return picks;
	}
};

} // namespace

TEST( ObjectFactoryTest, メダルは拾う物で回り_硬くなく当たりの応答を持たない )
{
	FactoryFixture f;

	const auto id = f.factory.create_object( "medal" );
	ASSERT_TRUE( id.has_value() );

	auto* e = f.entity( * id );
	ASSERT_NE( e->get_component< blue_sky::ecs::PickupComponent >(), nullptr );
	EXPECT_EQ( e->get_component< blue_sky::ecs::PickupComponent >()->kind, blue_sky::ecs::PickupKind::MEDAL );
	ASSERT_NE( e->get_component< blue_sky::ecs::SpinComponent >(), nullptr );
	EXPECT_EQ( e->get_component< blue_sky::ecs::StaticObjectComponent >(), nullptr );

	// 以前の Medal : 硬くない・足元と剛体の中心が同じ・重力 0・当たりの応答なし
	EXPECT_FALSE( e->get_component< blue_sky::ecs::CollisionTraitsComponent >()->is_hard );
	const auto* body = e->get_component< blue_sky::ecs::PhysicsBodyComponent >();
	EXPECT_FLOAT_EQ( body->height_offset, 0.f );
	EXPECT_NE( body->body->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE, 0 );
	EXPECT_TRUE( is_near( e->get_component< blue_sky::ecs::SimpleObjectComponent >()->default_gravity, Vector::Zero ) );

	// 1 秒に 90 度回る
	f.factory.restart();
	f.manager.get_system< blue_sky::ecs::SpinSystem >()->update_spins( 0.5f );
	EXPECT_FLOAT_EQ( e->get_component< blue_sky::ecs::SimpleObjectComponent >()->direction_degree, 45.f );
}

TEST( ObjectFactoryTest, プレイヤーがメダルに当たると拾って_メダルは死ぬ_拾わない物体なら何も起きない )
{
	FactoryFixture f;

	const auto id = * f.factory.create_object( "medal" );
	const auto* traits = f.entity( id )->get_component< blue_sky::ecs::CollisionTraitsComponent >();

	// 拾わない物体（敵など）: 何も起きない（以前も Medal との当たりは Player だけが受け取っていた）
	FakePicker robot;
	robot.picks = false;
	blue_sky::ActiveObjectManager::on_collide_with_entity_object( robot, FakeObjectId, * f.entity( id ) );
	EXPECT_FALSE( traits->is_dead );

	// プレイヤー : 拾い、メダルは死んで描かれなくなる
	FakePicker player;
	blue_sky::ActiveObjectManager::on_collide_with_entity_object( player, FakeObjectId, * f.entity( id ) );

	ASSERT_EQ( player.picked.size(), 1u );
	EXPECT_EQ( player.picked[ 0 ], blue_sky::ecs::PickupKind::MEDAL );
	EXPECT_TRUE( traits->is_dead );
	EXPECT_FALSE( f.entity( id )->get_component< blue_sky::ecs::ObjectRenderComponent >()->is_visible );

	// やり直すと戻る
	f.factory.restart();
	EXPECT_FALSE( traits->is_dead );
}

TEST( ObjectFactoryTest, ロケットは硬い拾う物で_拾われると当たりの応答が消え_やり直すと戻る )
{
	FactoryFixture f;

	const auto id = f.factory.create_object( "rocket" );
	ASSERT_TRUE( id.has_value() );

	auto* e = f.entity( * id );
	const auto* traits = e->get_component< blue_sky::ecs::CollisionTraitsComponent >();
	const auto* body = e->get_component< blue_sky::ecs::PhysicsBodyComponent >()->body;

	EXPECT_EQ( e->get_component< blue_sky::ecs::PickupComponent >()->kind, blue_sky::ecs::PickupKind::ROCKET );
	EXPECT_TRUE( traits->is_hard );
	EXPECT_FALSE( traits->is_block );
	EXPECT_EQ( body->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE, 0 );
	EXPECT_TRUE( is_near( e->get_component< blue_sky::ecs::SimpleObjectComponent >()->default_gravity, blue_sky::GameObject::GravityDefault ) );

	FakePicker player;
	blue_sky::ActiveObjectManager::on_collide_with_entity_object( player, FakeObjectId, * e );

	ASSERT_EQ( player.picked.size(), 1u );
	EXPECT_EQ( player.picked[ 0 ], blue_sky::ecs::PickupKind::ROCKET );
	EXPECT_TRUE( traits->is_dead );
	EXPECT_NE( body->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE, 0 );

	f.factory.restart();

	EXPECT_FALSE( traits->is_dead );
	EXPECT_EQ( body->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE, 0 );
}

TEST( ObjectFactoryTest, ゴールは死なない足場で重力が無く_やり直すと質量が_100_になり_当たりでは何も起こさない )
{
	FactoryFixture f;

	const auto id = f.factory.create_object( "goal" );
	ASSERT_TRUE( id.has_value() );

	auto* e = f.entity( * id );
	const auto* traits = e->get_component< blue_sky::ecs::CollisionTraitsComponent >();
	const auto* body = e->get_component< blue_sky::ecs::PhysicsBodyComponent >()->body;

	EXPECT_TRUE( traits->is_hard );
	EXPECT_TRUE( traits->is_safe_footing );
	EXPECT_EQ( e->get_component< blue_sky::ecs::PickupComponent >(), nullptr );
	EXPECT_EQ( e->get_component< blue_sky::ecs::StaticObjectComponent >(), nullptr );
	EXPECT_FLOAT_EQ( body->getInvMass(), 1.f );		// 作ったときは 1（以前と同じ）

	f.factory.restart();

	EXPECT_FLOAT_EQ( body->getInvMass(), 1.f / 100.f );
	EXPECT_NEAR( body->getGravity().length(), 0.f, 0.0001f );

	FakePicker player;
	blue_sky::ActiveObjectManager::on_collide_with_entity_object( player, FakeObjectId, * e );

	EXPECT_TRUE( player.picked.empty() );
	EXPECT_EQ( player.static_object_count, 0 );
	EXPECT_FALSE( traits->is_dead );
}

TEST( ObjectFactoryTest, 傘は硬くない拾う物で_プレイヤーが触れると消える )
{
	FactoryFixture f;

	const auto id = f.factory.create_object( "umbrella" );
	ASSERT_TRUE( id.has_value() );

	auto* e = f.entity( * id );
	ASSERT_NE( e->get_component< blue_sky::ecs::PickupComponent >(), nullptr );
	EXPECT_EQ( e->get_component< blue_sky::ecs::PickupComponent >()->kind, blue_sky::ecs::PickupKind::UMBRELLA );
	EXPECT_EQ( e->get_component< blue_sky::ecs::SpinComponent >(), nullptr );
	EXPECT_FALSE( e->get_component< blue_sky::ecs::CollisionTraitsComponent >()->is_hard );
	EXPECT_FLOAT_EQ( e->get_component< blue_sky::ecs::PhysicsBodyComponent >()->height_offset, 1.f );

	FakePicker player;
	blue_sky::ActiveObjectManager::on_collide_with_entity_object( player, FakeObjectId, * e );

	EXPECT_TRUE( e->get_component< blue_sky::ecs::CollisionTraitsComponent >()->is_dead );
}
