#include "pch.h"

#include <blue_sky/ObjectFactory.h>
#include <blue_sky/GameObjectRef.h>
#include <blue_sky/ecs/ContactRules.h>
#include <blue_sky/ecs/ObjectBody.h>
#include <blue_sky/ecs/component/PlayerBehaviorComponent.h>
#include <core/physics/PhysicsManager.h>

#include <blue_sky/ecs/EntityObject.h>
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
#include <blue_sky/ecs/system/PhysicsSyncSystem.h>
#include <blue_sky/ecs/system/BalloonHoldSystem.h>
#include <blue_sky/ecs/component/PickupComponent.h>

#include <core/ecs/EntityManager.h>
#include <core/ecs/component/TransformComponent.h>

#include <common/math.h>


#include <cmath>
#include <sstream>

/**
 * 段階 4 : 物体を種類の名前から作る ObjectFactory と、ECS だけの物体（動かない物・動く床）のテスト
 *
 * 描画（GraphicsManager）は無しで作る（モデル・定数バッファは用意しない）。物理の世界は本物（core::physics::PhysicsManager。以前は本編だけの ActiveObjectPhysics）を使う
 */

using blue_sky::ObjectFactory;
using blue_sky::GameObjectRef;
using core::ecs::EntityManager;
using core::Vector;

namespace
{

/// 描画なしで工場を作る
struct FactoryFixture
{
	core::physics::PhysicsManager physics;
	EntityManager manager;
	ObjectFactory factory{ & manager, & physics, nullptr };

	/// プレイヤー（ECS だけの物体。剛体なし）を作る（段階 6 までは GameObject の仮想関数を実装した偽物を使っていた）
	core::ecs::EntityId create_player() { return factory.create_player(); }
	blue_sky::ecs::PlayerBehaviorComponent& player_of( core::ecs::EntityId id ) { return * entity( id )->get_component< blue_sky::ecs::PlayerBehaviorComponent >(); }
	blue_sky::ecs::PlayerInventoryComponent& inventory_of( core::ecs::EntityId id ) { return * entity( id )->get_component< blue_sky::ecs::PlayerInventoryComponent >(); }

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

bool is_near( const Vector& a, const Vector& b, float_t e = 0.0001f )
{
	return std::abs( a.x() - b.x() ) < e && std::abs( a.y() - b.y() ) < e && std::abs( a.z() - b.z() ) < e;
}

} // namespace

// ---- 作った Entity が持つ部品 ----

TEST( ObjectFactoryTest, 動かない物は部品だけで作られる )
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

	// 目標へ動かす窓口は部品を直接動かす印の nullptr
	ASSERT_NE( e->get_component< blue_sky::ecs::TargetMoverComponent >(), nullptr );
	EXPECT_EQ( e->get_component< blue_sky::ecs::TargetMoverComponent >()->mover, nullptr );

	EXPECT_EQ( f.factory.get_entity_object_list().size(), 1u );
	EXPECT_TRUE( f.factory.get_ref( id ).is_alive() );
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
	EXPECT_FLOAT_EQ( wall_body->get()->get_inv_mass(), 1.f / 100.f );
	EXPECT_EQ( wall_body->get()->get_shape_type(), core::physics::RigidBodyShapeType::Box );

	// soda-can-1 だけ円柱
	const auto can = f.create_from_line( "soda-can-1 0 0 0" );
	EXPECT_EQ( f.entity( can )->get_component< blue_sky::ecs::PhysicsBodyComponent >()->get()->get_shape_type(), core::physics::RigidBodyShapeType::Cylinder );

	// box-2x2x2 は摩擦 10
	const auto box = f.create_from_line( "box-2x2x2 0 0 0" );
	EXPECT_FLOAT_EQ( f.entity( box )->get_component< blue_sky::ecs::PhysicsBodyComponent >()->get()->get_friction(), 10.f );

	// 知らない種類は大きさ 0・質量 0（動かない）
	const auto tree = f.create_from_line( "tree-1 0 0 0" );
	EXPECT_FLOAT_EQ( f.entity( tree )->get_component< blue_sky::ecs::PhysicsBodyComponent >()->get()->get_inv_mass(), 0.f );
}

TEST( ObjectFactoryTest, 剛体には_Entity_の番号のタグが付き_足元から高さの半分上に置かれる )
{
	FactoryFixture f;

	const auto id = f.create_from_line( "wall-3 1 2 3" );
	const auto* body = f.entity( id )->get_component< blue_sky::ecs::PhysicsBodyComponent >()->get();

	ASSERT_TRUE( body->get_entity_tag().has_value() );
	EXPECT_EQ( * body->get_entity_tag(), id );

	const Vector origin = body->get_world_transform( Vector::Zero ).get_position();
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

	// プレイヤーは create_player で作り、種類の名前からは作らない（以前もシーンが作っていた）
	EXPECT_FALSE( f.factory.create_object( "player" ).has_value() );
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

	const auto* body = f.entity( id )->get_component< blue_sky::ecs::PhysicsBodyComponent >()->get();
	EXPECT_NEAR( body->get_linear_velocity().length(), 0.f, 0.0001f );

	// 向き（Y 軸 90 度）が剛体にも送られている
	const core::Transform t = body->get_world_transform( Vector::Zero );
	EXPECT_NEAR( std::abs( t.forward().x() ), 1.f, 0.001f );		// 前が X 軸を向く
	EXPECT_NEAR( t.forward().z(), 0.f, 0.001f );
	EXPECT_NEAR( t.get_position().y(), 2.f + 1.25f, 0.0001f );
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

	const auto* body = f.entity( id )->get_component< blue_sky::ecs::PhysicsBodyComponent >()->get();

	// まだ遠い : 目標へ向けて速さ 1 の速度を設定する
	system->update_targets( 1.f / 60.f );
	EXPECT_NEAR( body->get_linear_velocity().y(), 1.f, 0.0001f );
	EXPECT_NE( f.entity( id )->get_component< blue_sky::ecs::TargetLocationComponent >(), nullptr );

	// 目標の近くに置くと、次のフレームで目標に置いて止め、目標を外す
	o.set_location( Vector( 0.f, 3.95f, 0.f ) );
	system->update_targets( 1.f / 60.f );

	EXPECT_TRUE( is_near( o.get_location(), Vector( 0.f, 4.f, 0.f ) ) );
	EXPECT_NEAR( body->get_linear_velocity().length(), 0.f, 0.0001f );
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

// ---- 当たり（部品の組み合わせで決める） ----

TEST( ObjectFactoryTest, 動かない物に当たるとプレイヤーに知らせ_ロケットで飛んでいれば止まる_印の無い物には何もしない )
{
	FactoryFixture f;
	const auto player = f.create_player();

	const auto wall = f.create_from_line( "wall-3 0 0 0" );

	f.player_of( player ).action_mode = blue_sky::ecs::PlayerBehaviorComponent::ActionMode::ROCKET;
	blue_sky::ecs::contact_rules::on_collide( * f.entity( player ), * f.entity( wall ) );

	EXPECT_EQ( f.player_of( player ).action_mode, blue_sky::ecs::PlayerBehaviorComponent::ActionMode::NONE );

	// 動かない物の印が無い Entity
	f.player_of( player ).action_mode = blue_sky::ecs::PlayerBehaviorComponent::ActionMode::ROCKET;
	auto* other = f.manager.create_entity();
	blue_sky::ecs::contact_rules::on_collide( * f.entity( player ), * other );

	EXPECT_EQ( f.player_of( player ).action_mode, blue_sky::ecs::PlayerBehaviorComponent::ActionMode::ROCKET );
}

// ---- 拾う物（段階 4 ②） ----

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
	EXPECT_FALSE( body->get()->has_contact_response() );
	EXPECT_TRUE( is_near( e->get_component< blue_sky::ecs::SimpleObjectComponent >()->default_gravity, Vector::Zero ) );

	// 1 秒に 90 度回る
	f.factory.restart();
	f.manager.get_system< blue_sky::ecs::SpinSystem >()->update_spins( 0.5f );
	EXPECT_FLOAT_EQ( e->get_component< blue_sky::ecs::SimpleObjectComponent >()->direction_degree, 45.f );
}

TEST( ObjectFactoryTest, メダルと風船は位置で動かす剛体で_物理を進めても書いた位置と向きのまま_プレイヤーとの当たりは起きる )
{
	FactoryFixture f;

	const auto medal = * f.factory.create_object( "medal", Vector( 0.f, 5.f, 0.f ) );
	const auto balloon = * f.factory.create_object( "balloon", Vector( 10.f, 5.f, 0.f ) );
	const auto stone = * f.factory.create_object( "stone", Vector( 20.f, 5.f, 0.f ) );
	const auto robot = * f.factory.create_object( "robot", Vector( 30.f, 5.f, 0.f ) );

	auto body_of = [ & ] ( core::ecs::EntityId id ) { return f.entity( id )->get_component< blue_sky::ecs::PhysicsBodyComponent >()->get(); };

	// System が位置を書く物だけがキネマティック（物理で動く石・敵は今までどおり）
	EXPECT_TRUE( body_of( medal )->is_kinematic() );
	EXPECT_TRUE( body_of( balloon )->is_kinematic() );
	EXPECT_FALSE( body_of( stone )->is_kinematic() );
	EXPECT_FALSE( body_of( robot )->is_kinematic() );

	f.factory.restart();

	// 向きを書いてから物理を何度進めても、書いた位置と向きのまま（Bullet が動きの状態から読み直しても戻らない）
	f.manager.get_system< blue_sky::ecs::SpinSystem >()->update_spins( 0.5f );
	blue_sky::ecs::EntityObject( f.entity( balloon ) ).set_location( Vector( 10.f, 6.f, 0.f ) );

	for ( int n = 0; n < 10; n++ )
	{
		f.physics.step_simulation( 1.f / 60.f );
	}

	f.manager.get_system< blue_sky::ecs::PhysicsSyncSystem >()->update_transforms();

	const blue_sky::ecs::EntityObject m( f.entity( medal ) );
	EXPECT_TRUE( is_near( m.get_location(), Vector( 0.f, 5.f, 0.f ) ) );

	float yaw = 0.f, pitch = 0.f, roll = 0.f;
	m.get_transform().get_rotation().get_yaw_pitch_roll( yaw, pitch, roll );
	EXPECT_NEAR( yaw, 3.14159265f / 4.f, 1e-4f );		// 0.5 秒で 45 度（剛体から読み直しても同じ）

	EXPECT_TRUE( is_near( blue_sky::ecs::EntityObject( f.entity( balloon ) ).get_location(), Vector( 10.f, 6.f, 0.f ) ) );

	// 石（動く剛体）は重力で落ちる
	EXPECT_LT( blue_sky::ecs::EntityObject( f.entity( stone ) ).get_location().y(), 5.f );

	// プレイヤー（動く剛体）をメダルに重ねると、当たりの組にメダルが入る
	const auto player = f.create_player();
	blue_sky::ecs::EntityObject( f.entity( player ) ).set_start_location( Vector( 0.f, 5.f, 0.f ) );
	f.factory.add_player_rigid_body( player );

	std::vector< std::optional< core::ecs::EntityId > > touched;
	f.physics.contact_test( * body_of( player ), [ & ] ( std::optional< core::ecs::EntityId >, std::optional< core::ecs::EntityId > b ) { touched.push_back( b ); } );
	EXPECT_NE( std::find( touched.begin(), touched.end(), std::optional< core::ecs::EntityId >( medal ) ), touched.end() );

	// 物理を進めると、プレイヤーとメダルの組が世界の接点に入る（当たりの収集 check_collision_all が受け取る）
	f.physics.step_simulation( 1.f / 60.f );

	bool found = false;
	f.physics.for_each_penetrating_contact( [ & ] ( std::optional< core::ecs::EntityId > a, std::optional< core::ecs::EntityId > b )
	{
		found = found || ( a == player && b == medal ) || ( a == medal && b == player );
	} );
	EXPECT_TRUE( found );
}

TEST( ObjectFactoryTest, プレイヤーがメダルに当たると拾って_メダルは死ぬ_拾わない物体なら何も起きない )
{
	FactoryFixture f;

	const auto id = * f.factory.create_object( "medal" );
	const auto* traits = f.entity( id )->get_component< blue_sky::ecs::CollisionTraitsComponent >();

	// プレイヤーでない物体（敵）: 何も起きない（以前も Medal との当たりは Player だけが受け取っていた）
	const auto robot = * f.factory.create_object( "robot" );
	blue_sky::ecs::contact_rules::on_collide( * f.entity( robot ), * f.entity( id ) );
	EXPECT_FALSE( traits->is_dead );

	// プレイヤー : 拾い、メダルは死んで描かれなくなる
	const auto player = f.create_player();
	blue_sky::ecs::contact_rules::on_collide( * f.entity( player ), * f.entity( id ) );

	EXPECT_TRUE( f.inventory_of( player ).has_medal );
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
	const auto* body = e->get_component< blue_sky::ecs::PhysicsBodyComponent >()->get();

	EXPECT_EQ( e->get_component< blue_sky::ecs::PickupComponent >()->kind, blue_sky::ecs::PickupKind::ROCKET );
	EXPECT_TRUE( traits->is_hard );
	EXPECT_FALSE( traits->is_block );
	EXPECT_TRUE( body->has_contact_response() );
	EXPECT_TRUE( is_near( e->get_component< blue_sky::ecs::SimpleObjectComponent >()->default_gravity, blue_sky::ecs::object_body::GravityDefault ) );

	const auto player = f.create_player();
	blue_sky::ecs::contact_rules::on_collide( * e, * f.entity( player ) );

	EXPECT_EQ( f.inventory_of( player ).get_item_count( blue_sky::ecs::PlayerBehaviorComponent::ItemType::ROCKET ), 1 );
	EXPECT_TRUE( traits->is_dead );
	EXPECT_FALSE( body->has_contact_response() );

	f.factory.restart();

	EXPECT_FALSE( traits->is_dead );
	EXPECT_TRUE( body->has_contact_response() );
}

TEST( ObjectFactoryTest, ゴールは死なない足場で重力が無く_やり直すと質量が_100_になり_当たりでは何も起こさない )
{
	FactoryFixture f;

	const auto id = f.factory.create_object( "goal" );
	ASSERT_TRUE( id.has_value() );

	auto* e = f.entity( * id );
	const auto* traits = e->get_component< blue_sky::ecs::CollisionTraitsComponent >();
	const auto* body = e->get_component< blue_sky::ecs::PhysicsBodyComponent >()->get();

	EXPECT_TRUE( traits->is_hard );
	EXPECT_TRUE( traits->is_safe_footing );
	EXPECT_EQ( e->get_component< blue_sky::ecs::PickupComponent >(), nullptr );
	EXPECT_EQ( e->get_component< blue_sky::ecs::StaticObjectComponent >(), nullptr );
	EXPECT_FLOAT_EQ( body->get_inv_mass(), 1.f );		// 作ったときは 1（以前と同じ）

	f.factory.restart();

	EXPECT_FLOAT_EQ( body->get_inv_mass(), 1.f / 100.f );
	EXPECT_NEAR( body->get_gravity().length(), 0.f, 0.0001f );

	// プレイヤーが当たっても何も起こさない（拾わない・ロケットも止めない。ゴールしたかはシーンがプレイヤーとの接触で調べる）
	const auto player = f.create_player();
	f.player_of( player ).action_mode = blue_sky::ecs::PlayerBehaviorComponent::ActionMode::ROCKET;
	blue_sky::ecs::contact_rules::on_collide( * f.entity( player ), * e );

	EXPECT_EQ( f.player_of( player ).action_mode, blue_sky::ecs::PlayerBehaviorComponent::ActionMode::ROCKET );
	EXPECT_FALSE( f.inventory_of( player ).has_medal );
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

	// 以前から、プレイヤーの持ち物は増えない
	const auto player = f.create_player();
	blue_sky::ecs::contact_rules::on_collide( * f.entity( player ), * e );

	EXPECT_TRUE( e->get_component< blue_sky::ecs::CollisionTraitsComponent >()->is_dead );
	EXPECT_EQ( f.inventory_of( player ).get_item_count( blue_sky::ecs::PlayerBehaviorComponent::ItemType::UMBRELLA ), 0 );
}
