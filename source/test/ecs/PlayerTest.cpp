#include "pch.h"

#include <blue_sky/ObjectFactory.h>
#include <blue_sky/GameObjectRef.h>
#include <blue_sky/ActiveObjectManager.h>
#include <blue_sky/ActiveObjectPhysics.h>

#include <blue_sky/ecs/EntityObject.h>
#include <blue_sky/ecs/component/CollisionTraitsComponent.h>
#include <blue_sky/ecs/component/ObjectRenderComponent.h>
#include <blue_sky/ecs/component/PhysicsBodyComponent.h>
#include <blue_sky/ecs/component/SimpleObjectComponent.h>
#include <blue_sky/ecs/component/FlickerComponent.h>
#include <blue_sky/ecs/component/PickupComponent.h>
#include <blue_sky/ecs/component/PlayerBehaviorComponent.h>
#include <blue_sky/ecs/component/PlayerInputComponent.h>
#include <blue_sky/ecs/system/PlayerBehaviorSystem.h>
#include <blue_sky/ecs/system/PlayerInputSystem.h>

#include <core/ecs/EntityManager.h>
#include <core/physics/EntityTag.h>

#include <btBulletDynamicsCommon.h>

#include <cmath>

/**
 * 段階 6 : プレイヤーを ECS だけの物体（行動をまとめた部品 PlayerBehaviorComponent・入力の部品 PlayerInputComponent）にしたときのテスト
 *
 * 描画（GraphicsManager）・音は無しで作る。物理の世界は本物（ActiveObjectPhysics）を使うが、地面は置かない（足場のレイは何にも当たらない）。
 * 入力は偽の入力（PlayerInputComponent に直接書く）
 */

using blue_sky::ObjectFactory;
using blue_sky::GameObjectRef;
using blue_sky::ecs::PlayerBehaviorComponent;
using blue_sky::ecs::PlayerInputComponent;
using blue_sky::ecs::PlayerInputSystem;
using core::ecs::EntityManager;
using core::ecs::EntityId;
using core::Vector;

namespace
{

using ActionMode = PlayerBehaviorComponent::ActionMode;
using ItemType = PlayerBehaviorComponent::ItemType;

/// 描画なしで工場を作り、プレイヤーを (0, 10, 0) に置いて剛体を作り、やり直す
struct Fixture
{
	blue_sky::ActiveObjectPhysics physics;
	EntityManager manager;
	ObjectFactory factory{ & manager, nullptr, & physics, nullptr };
	EntityId player = 0;

	Fixture()
	{
		player = factory.create_player();
		factory.get_ref( player ).set_start_location( 0.f, 10.f, 0.f );
		factory.add_player_rigid_body( player );
		factory.restart();
	}

	~Fixture()
	{
		// 剛体の持ち主（部品）を、物理の世界より先に消す
		factory.clear();
		manager.clear();
	}

	core::ecs::Entity* entity( EntityId id ) { return manager.find( id ); }
	PlayerBehaviorComponent& p() { return * entity( player )->get_component< PlayerBehaviorComponent >(); }
	PlayerInputComponent& input() { return * entity( player )->get_component< PlayerInputComponent >(); }
	blue_sky::ecs::PlayerBehaviorSystem* behavior() { return manager.get_system< blue_sky::ecs::PlayerBehaviorSystem >(); }
	PlayerInputSystem* input_system() { return manager.get_system< PlayerInputSystem >(); }
	btRigidBody* body() { return entity( player )->get_component< blue_sky::ecs::PhysicsBodyComponent >()->body; }
	Vector velocity() { return factory.get_ref( player ).get_velocity(); }

	/// 偽の入力を書いて 1 フレームぶん操作する
	PlayerInputSystem::Result operate( const PlayerInputComponent& in, const PlayerInputSystem::View& view = PlayerInputSystem::View() )
	{
		input() = in;
		return input_system()->update_player( player, view );
	}

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

TEST( PlayerTest, プレイヤーは硬い物で_剛体はカプセル_やり直すと質量_50_回らず摩擦も無く_状態が戻る )
{
	Fixture f;
	auto* e = f.entity( f.player );

	ASSERT_NE( e->get_component< PlayerBehaviorComponent >(), nullptr );
	ASSERT_NE( e->get_component< PlayerInputComponent >(), nullptr );
	ASSERT_NE( e->get_component< blue_sky::ecs::FlickerComponent >(), nullptr );
	EXPECT_EQ( e->get_component< blue_sky::ecs::SimpleObjectComponent >()->kind, "player" );

	const auto* traits = e->get_component< blue_sky::ecs::CollisionTraitsComponent >();
	EXPECT_TRUE( traits->is_hard );
	EXPECT_TRUE( traits->is_block );
	EXPECT_FALSE( traits->is_safe_footing );

	// 以前の add_active_object_as_capsule : 足元から剛体の中心まで 0.875・Entity の番号のタグ
	const auto* body = e->get_component< blue_sky::ecs::PhysicsBodyComponent >();
	EXPECT_FLOAT_EQ( body->height_offset, 0.875f );
	EXPECT_EQ( core::physics::get_entity_tag( body->body ), std::optional< EntityId >( f.player ) );

	// 以前の Player::restart
	EXPECT_FLOAT_EQ( body->body->getInvMass(), 1.f / 50.f );
	EXPECT_FLOAT_EQ( body->body->getFriction(), 0.f );
	EXPECT_TRUE( is_near( Vector( body->body->getAngularFactor() ), Vector::Zero ) );
	EXPECT_TRUE( is_near( Vector( body->body->getGravity() ), Vector( 0.f, -9.8f, 0.f ) ) );

	EXPECT_EQ( f.p().get_item_count( ItemType::SCOPE ), 1 );
	EXPECT_EQ( f.p().selected_item_type, ItemType::NONE );
	EXPECT_EQ( f.p().hp, 1 );
	EXPECT_FLOAT_EQ( f.p().eye_height, 1.5f );
	EXPECT_FLOAT_EQ( f.p().last_footing_height, 10.f );

	// 持ち物・状態はやり直すと戻る
	f.p().has_medal = true;
	f.p().item_count[ static_cast< int >( ItemType::STONE ) ] = 2;
	f.p().action_mode = ActionMode::BALLOON;
	f.factory.restart();

	EXPECT_FALSE( f.p().has_medal );
	EXPECT_EQ( f.p().get_item_count( ItemType::STONE ), 0 );
	EXPECT_EQ( f.p().action_mode, ActionMode::NONE );
}

TEST( PlayerTest, 前を押すと前へ歩く速さぶん速度が増え_離すと歩く速さが半分になる )
{
	Fixture f;

	PlayerInputComponent in;
	in.up = true;
	f.operate( in );

	// 向き 0 の前は +Z。歩き始めの速さは 0.25（以前の Player::step）
	EXPECT_TRUE( is_near( f.velocity(), Vector( 0.f, 0.f, 0.25f ) ) );
	EXPECT_EQ( f.p().step_count, 1 );

	// 離すと止まる（歩数 0・歩く速さは毎フレーム半分）
	f.operate( PlayerInputComponent() );
	EXPECT_EQ( f.p().step_count, 0 );
	EXPECT_FLOAT_EQ( f.p().step_speed, 0.125f );

	// 横歩きは右へ 0.25
	f.body()->setLinearVelocity( btVector3( 0, 0, 0 ) );
	in = PlayerInputComponent();
	in.right = true;
	f.operate( in );
	EXPECT_TRUE( is_near( f.velocity(), Vector( 0.25f, 0.f, 0.f ) ) );
}

TEST( PlayerTest, マウスの横の移動で向きが変わり_ロケットで飛んでいる間は歩けず向きも変わらない )
{
	Fixture f;

	PlayerInputComponent in;
	in.mouse_dx = 0.1f;

	PlayerInputSystem::View view;
	view.rotation_speed_rate = 0.5f;
	f.operate( in, view );

	// 0.1 x 90 度 x 0.5（ズーム中の割合）
	EXPECT_FLOAT_EQ( f.factory.get_ref( f.player ).get_direction_degree(), 4.5f );

	f.p().action_mode = ActionMode::ROCKET;
	in.up = true;
	f.body()->setLinearVelocity( btVector3( 0, 0, 0 ) );
	f.operate( in, view );

	EXPECT_FLOAT_EQ( f.factory.get_ref( f.player ).get_direction_degree(), 4.5f );
	EXPECT_TRUE( is_near( f.velocity(), Vector::Zero ) );
}

TEST( PlayerTest, A_を押すと選んだ持ち物で跳ぶ_望遠鏡をのぞく_ロケットで飛ぶ )
{
	Fixture f;

	PlayerInputComponent push_a;
	push_a.a = true;
	push_a.a_pushed = true;

	// 何も選んでいない : 跳べるときだけ上へ 4 跳ぶ
	f.operate( push_a );
	EXPECT_FALSE( f.p().is_jumping );

	f.p().is_jumpable = true;
	f.operate( push_a );
	EXPECT_TRUE( f.p().is_jumping );
	EXPECT_NEAR( f.velocity().y(), 4.f, 0.0001f );

	// 望遠鏡 : のぞき始め、ホイールは視野を変える。もう一度 A でやめる（選ぶのも戻る）
	f.p().selected_item_type = ItemType::SCOPE;
	auto result = f.operate( push_a );
	EXPECT_TRUE( result.scope_started );
	EXPECT_FALSE( result.reset_fov );
	EXPECT_EQ( f.p().action_mode, ActionMode::SCOPE );

	PlayerInputComponent wheel;
	wheel.wheel = 1;
	result = f.operate( wheel );
	EXPECT_EQ( result.scope_wheel, 1 );
	EXPECT_EQ( f.p().selected_item_type, ItemType::SCOPE );

	result = f.operate( push_a );
	EXPECT_FALSE( result.scope_started );
	EXPECT_TRUE( result.reset_fov );
	EXPECT_EQ( f.p().action_mode, ActionMode::NONE );
	EXPECT_EQ( f.p().selected_item_type, ItemType::NONE );

	// ロケット : カメラの前へ秒速 10 で飛び、持ち物が減る（無くなると選ぶのも戻る）
	f.p().item_count[ static_cast< int >( ItemType::ROCKET ) ] = 1;
	f.p().selected_item_type = ItemType::ROCKET;

	PlayerInputSystem::View view;
	view.camera_front = Vector( 1.f, 0.f, 0.f );
	f.operate( push_a, view );

	EXPECT_EQ( f.p().action_mode, ActionMode::ROCKET );
	EXPECT_TRUE( is_near( f.velocity(), Vector( 10.f, 0.f, 0.f ) ) );
	EXPECT_EQ( f.p().get_item_count( ItemType::ROCKET ), 0 );
	EXPECT_EQ( f.p().selected_item_type, ItemType::NONE );
}

TEST( PlayerTest, ホイールで持っている物を順に選ぶ )
{
	Fixture f;

	f.p().item_count[ static_cast< int >( ItemType::STONE ) ] = 1;

	PlayerInputComponent next;
	next.wheel = 1;
	PlayerInputComponent prev;
	prev.wheel = -1;

	// 持っていない物（ロケット・傘）は飛ばす
	f.operate( next );
	EXPECT_EQ( f.p().selected_item_type, ItemType::STONE );
	f.operate( next );
	EXPECT_EQ( f.p().selected_item_type, ItemType::SCOPE );
	f.operate( next );
	EXPECT_EQ( f.p().selected_item_type, ItemType::SCOPE );

	f.operate( prev );
	EXPECT_EQ( f.p().selected_item_type, ItemType::STONE );
	f.operate( prev );
	EXPECT_EQ( f.p().selected_item_type, ItemType::NONE );
}

TEST( PlayerTest, 見下ろす角度が上下の向きになり_死んでいる間は操作せず_A_でやり直しを頼む )
{
	Fixture f;

	PlayerInputSystem::View view;
	view.camera_rotate_x = 45.f;
	f.operate( PlayerInputComponent(), view );
	EXPECT_FLOAT_EQ( f.p().pitch, -0.5f );

	f.behavior()->kill( f.player );

	PlayerInputComponent in;
	in.up = true;
	auto result = f.operate( in );

	EXPECT_TRUE( result.is_dead );
	EXPECT_FALSE( result.restart_requested );
	EXPECT_EQ( f.p().step_count, 0 );

	in.a = true;
	in.a_pushed = true;
	result = f.operate( in );
	EXPECT_TRUE( result.restart_requested );
}

TEST( PlayerTest, 死ぬと描かず当たりも起こさず倒れるようになり_スクリプトの_kill_も同じ )
{
	Fixture f;

	// スクリプトの kill（GameObjectRef → EntityObject::kill）もプレイヤーの死に方になる（以前は仮想関数の Player::kill）
	f.factory.get_ref( f.player ).kill();

	EXPECT_TRUE( f.behavior()->is_dead( f.player ) );
	EXPECT_TRUE( f.entity( f.player )->get_component< blue_sky::ecs::CollisionTraitsComponent >()->is_dead );
	EXPECT_FALSE( f.entity( f.player )->get_component< blue_sky::ecs::ObjectRenderComponent >()->is_visible );
	EXPECT_TRUE( is_near( Vector( f.body()->getAngularFactor() ), Vector( 1.f, 1.f, 1.f ) ) );
	EXPECT_FLOAT_EQ( f.body()->getFriction(), 1.f );

	// やり直すと生き返る
	f.factory.restart();
	EXPECT_FALSE( f.behavior()->is_dead( f.player ) );
	EXPECT_TRUE( f.entity( f.player )->get_component< blue_sky::ecs::ObjectRenderComponent >()->is_visible );
}

TEST( PlayerTest, メダル・ロケット・石に触れると拾い_拾われた物は死ぬ )
{
	Fixture f;

	const auto medal = f.create( "medal" );
	const auto rocket = f.create( "rocket" );
	const auto stone = f.create( "stone" );
	f.factory.restart();

	blue_sky::ActiveObjectManager::on_collide_entity_objects( * f.entity( medal ), * f.entity( f.player ) );
	blue_sky::ActiveObjectManager::on_collide_entity_objects( * f.entity( f.player ), * f.entity( rocket ) );

	EXPECT_TRUE( f.p().has_medal );
	EXPECT_TRUE( blue_sky::ecs::EntityObject( f.entity( medal ) ).is_dead() );
	EXPECT_EQ( f.p().get_item_count( ItemType::ROCKET ), 1 );
	EXPECT_EQ( f.p().selected_item_type, ItemType::ROCKET );
	EXPECT_TRUE( blue_sky::ecs::EntityObject( f.entity( rocket ) ).is_dead() );

	blue_sky::ActiveObjectManager::on_collide_entity_objects( * f.entity( stone ), * f.entity( f.player ) );

	EXPECT_EQ( f.p().get_item_count( ItemType::STONE ), 1 );
	EXPECT_EQ( f.p().selected_item_type, ItemType::STONE );
	ASSERT_EQ( f.p().stone_list.size(), 1u );
	EXPECT_EQ( f.p().stone_list.front().get_entity_id(), stone );
	EXPECT_TRUE( blue_sky::ecs::EntityObject( f.entity( stone ) ).is_dead() );
}

TEST( PlayerTest, 石を投げると石は生き返り_目の高さの前から投げた向きへ秒速_8_で飛ぶ )
{
	Fixture f;

	const auto stone = f.create( "stone" );
	f.factory.restart();

	blue_sky::ActiveObjectManager::on_collide_entity_objects( * f.entity( f.player ), * f.entity( stone ) );
	ASSERT_EQ( f.p().get_item_count( ItemType::STONE ), 1 );

	// 前に壁があると投げられない
	f.p().can_throw = false;
	f.behavior()->throw_stone( f.player, Vector( 0.f, 0.f, 1.f ) );
	EXPECT_EQ( f.p().get_item_count( ItemType::STONE ), 1 );

	f.p().can_throw = true;
	f.behavior()->throw_stone( f.player, Vector( 0.f, 0.f, 1.f ) );

	EXPECT_EQ( f.p().get_item_count( ItemType::STONE ), 0 );
	EXPECT_EQ( f.p().selected_item_type, ItemType::NONE );
	EXPECT_TRUE( f.p().stone_list.empty() );

	const GameObjectRef s = f.factory.get_ref( stone );
	EXPECT_FALSE( blue_sky::ecs::EntityObject( f.entity( stone ) ).is_dead() );
	EXPECT_TRUE( is_near( s.get_location(), Vector( 0.f, 10.f + 1.5f, 0.5f ), 0.001f ) );
	EXPECT_TRUE( is_near( s.get_velocity(), Vector( 0.f, 0.f, 8.f ), 0.001f ) );
}

TEST( PlayerTest, 梯子に触れると梯子につかまり重力が無くなり_更新の終わりに忘れる_ロケットは動かない物と梯子で止まる )
{
	Fixture f;

	const auto ladder = f.create( "ladder" );
	const auto wall = f.create( "static" );
	f.factory.restart();

	blue_sky::ActiveObjectManager::on_collide_entity_objects( * f.entity( ladder ), * f.entity( f.player ) );
	EXPECT_TRUE( f.p().is_on_ladder );

	f.behavior()->update_player( f.player, 0.1f, 1.f );
	EXPECT_TRUE( is_near( Vector( f.body()->getGravity() ), Vector::Zero ) );
	EXPECT_FALSE( f.p().is_on_ladder );

	// 次のフレームに触れていなければ重力が戻る
	f.behavior()->update_player( f.player, 0.1f, 1.f );
	EXPECT_TRUE( is_near( Vector( f.body()->getGravity() ), Vector( 0.f, -9.8f, 0.f ) ) );

	// ロケットで飛んでいるときに動かない物に当たると止まる（跳んでいる扱い）
	f.p().action_mode = ActionMode::ROCKET;
	blue_sky::ActiveObjectManager::on_collide_entity_objects( * f.entity( f.player ), * f.entity( wall ) );
	EXPECT_EQ( f.p().action_mode, ActionMode::NONE );
	EXPECT_TRUE( f.p().is_jumping );

	f.p().action_mode = ActionMode::ROCKET;
	blue_sky::ActiveObjectManager::on_collide_entity_objects( * f.entity( f.player ), * f.entity( ladder ) );
	EXPECT_EQ( f.p().action_mode, ActionMode::NONE );
}

TEST( PlayerTest, 風船の動作では上へ秒速_3_まで上がり_5_m_上がると終わって跳ぶ )
{
	Fixture f;

	const auto balloon = f.create( "balloon" );
	f.factory.restart();

	blue_sky::ActiveObjectManager::on_collide_entity_objects( * f.entity( f.player ), * f.entity( balloon ) );
	ASSERT_EQ( f.p().action_mode, ActionMode::BALLOON );
	EXPECT_EQ( f.p().pop_balloon_sound_request(), 1 );
	EXPECT_EQ( f.p().pop_balloon_sound_request(), 0 );

	// 上がる速さは 0 から 0.25 ずつ 3 まで（以前の Player::update の風船）
	f.behavior()->update_player( f.player, 0.1f, 1.f );
	EXPECT_NEAR( f.velocity().y(), 0.25f, 0.0001f );
	EXPECT_TRUE( is_near( Vector( f.body()->getGravity() ), Vector::Zero ) );

	// 4 m（0.75）上がると終わりかけ、5 m 上がると終わって跳ぶ
	f.factory.get_ref( f.player ).set_location( Vector( 0.f, 14.f, 0.f ) );
	f.behavior()->update_player( f.player, 0.1f, 1.f );
	EXPECT_TRUE( f.p().is_action_pre_finish );

	f.factory.get_ref( f.player ).set_location( Vector( 0.f, 15.f, 0.f ) );
	f.behavior()->update_player( f.player, 0.1f, 1.f );
	EXPECT_EQ( f.p().action_mode, ActionMode::NONE );
	EXPECT_TRUE( f.p().is_jumping );
	EXPECT_FALSE( f.p().is_action_pre_finish );
}
