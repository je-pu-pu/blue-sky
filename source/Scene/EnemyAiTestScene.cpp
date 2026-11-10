#include "EnemyAiTestScene.h"

#include <blue_sky/SceneManager.h>
#include <blue_sky/GameMain.h>
#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/Input.h>

#include <core/ecs/EntityManager.h>
#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/component/CameraComponent.h>
#include <core/ecs/component/ModelComponent.h>
#include <core/ecs/component/RenderComponent.h>
#include <core/ecs/component/RigidBodyComponent.h>

#include <blue_sky/ecs/component/FpsPlayerComponent.h>
#include <blue_sky/ecs/component/PerceivableComponent.h>
#include <blue_sky/ecs/component/PerceptionComponent.h>
#include <blue_sky/ecs/component/AlertComponent.h>
#include <blue_sky/ecs/component/PatrolComponent.h>

#include <core/ecs/system/RenderSystem.h>
#include <core/ecs/system/PhysicsSystem.h>
#include <blue_sky/ecs/system/FpsPlayerControlSystem.h>
#include <blue_sky/ecs/system/TargetRegistrySystem.h>
#include <blue_sky/ecs/system/PerceptionSystem.h>
#include <blue_sky/ecs/system/AlertSystem.h>
#include <blue_sky/ecs/system/PatrolSystem.h>

#include <blue_sky/ai/EnemyPerception.h>

#include <core/physics/PhysicsManager.h>
#include <core/graphics/Direct3D11/BulletDebugDraw.h>

#include <game/MainLoop.h>

#include <sstream>
#include <cmath>

namespace blue_sky
{

using RigidBodyShapeType = core::physics::RigidBodyShapeType;
using core::ecs::TransformComponent;
using core::ecs::CameraComponent;
using core::ecs::ModelComponent;
using core::ecs::RenderComponent;
using core::ecs::RigidBodyComponent;

namespace
{

/// 警戒状態ごとの色（白/灰＝無警戒, 黄＝不審, 橙＝捜索, 赤＝警報）
core::Vector state_color( ai::AlertState s )
{
	switch ( s )
	{
	case ai::AlertState::Suspicious: return core::Vector( 1.f, 1.f, 0.f );
	case ai::AlertState::Searching:  return core::Vector( 1.f, 0.5f, 0.f );
	case ai::AlertState::Alarmed:    return core::Vector( 1.f, 0.f, 0.f );
	case ai::AlertState::Unaware:
	default:                         return core::Vector( 0.5f, 0.5f, 0.5f );
	}
}

const char* state_name( ai::AlertState s )
{
	switch ( s )
	{
	case ai::AlertState::Suspicious: return "SUSPICIOUS";
	case ai::AlertState::Searching:  return "SEARCHING";
	case ai::AlertState::Alarmed:    return "ALARMED";
	case ai::AlertState::Unaware:
	default:                         return "UNAWARE";
	}
}

} // namespace

EnemyAiTestScene::EnemyAiTestScene( GameMain& game_main )
	: Scene( game_main )
{
	get_graphics_manager()->setup_default_shaders();
	get_graphics_manager()->load_paper_textures();

	// Bullet デバッグ描画（LOS レイの遮蔽やコーン可視化に使う）
	get_physics_manager()->set_debug_drawer( get_bullet_debug_draw() );

	// System を追加（実行順が重要: 対象収集 → 知覚 → 警戒 → 移動 → 物理 → 描画）
	get_entity_manager()->add_system< core::ecs::PhysicsSystem >( 5, get_physics_manager() );
	get_entity_manager()->add_system< blue_sky::ecs::FpsPlayerControlSystem >( 10, get_input(), get_physics_manager() );
	get_entity_manager()->add_system< blue_sky::ecs::TargetRegistrySystem >( 20 );
	get_entity_manager()->add_system< blue_sky::ecs::PerceptionSystem >( 30, get_physics_manager() );
	get_entity_manager()->add_system< blue_sky::ecs::AlertSystem >( 40, get_time_manager() );
	get_entity_manager()->add_system< blue_sky::ecs::PatrolSystem >( 50, get_time_manager() );
	get_entity_manager()->add_system< core::ecs::RenderSystem >( 1000, get_graphics_manager(), get_time_manager() );

	// --- プレイヤー（侵入者） ---
	player_ = get_entity_manager()->create_entity();
	player_transform_ = player_->add_component< TransformComponent >();
	player_transform_->transform.set_identity();
	player_transform_->transform.set_position( Vector( 0.f, 2.f, -10.f ) );

	fps_player_ = player_->add_component< blue_sky::ecs::FpsPlayerComponent >();
	fps_player_->move_speed = 5.f;

	auto* player_rb = player_->add_component< RigidBodyComponent >();
	player_rb->shape_type = RigidBodyShapeType::Capsule;
	player_rb->shape_size = Vector( 0.3f, 1.0f, 0.f );
	player_rb->offset = Vector( 0.f, 0.8f, 0.f );
	player_rb->mass = 70.f;
	player_rb->angular_factor = Vector( 0.f, 0.f, 0.f );
	player_rb->friction = 0.1f;
	player_rb->collision_group = core::physics::CollisionGroup::Player;
	player_rb->collision_mask = core::physics::CollisionGroup::All;

	player_camera_ = player_->add_component< CameraComponent >();
	player_camera_->eye_offset = Vector( 0.f, 1.5f, 0.f );

	// プレイヤーを「知覚され得る対象」にする（敵はプレイヤー型を知らず、このタグで見る）
	player_->add_component< blue_sky::ecs::PerceivableComponent >();

	get_graphics_manager()->set_main_camera_info( player_transform_, player_camera_ );

	// --- 地面 ---
	{
		auto* e = get_entity_manager()->create_entity();
		auto* t = e->add_component< TransformComponent >();
		t->transform.set_identity();
		e->add_component< RenderComponent >();
		auto* m = e->add_component< ModelComponent >();
		m->model = get_graphics_manager()->get_model_handle( get_graphics_manager()->load_model( "ground" ) );

		auto* rb = e->add_component< RigidBodyComponent >();
		rb->shape_type = RigidBodyShapeType::Box;
		rb->shape_size = Vector( 500.f, 1.f, 500.f );
		rb->offset = Vector( 0.f, -1.f, 0.f );
		rb->mass = 0.f;
	}

	// --- 遮蔽ビル（LOS を切るための静的ボックス） ---
	const Vector building_positions[] = {
		Vector(  6.f, 0.f, 4.f ),
		Vector( -6.f, 0.f, 8.f ),
		Vector(  0.f, 0.f, 14.f ),
	};
	const auto box_model = get_graphics_manager()->get_model_handle( get_graphics_manager()->load_model( "box-1x1x1" ) );
	for ( const auto& p : building_positions )
	{
		auto* e = get_entity_manager()->create_entity();
		auto* t = e->add_component< TransformComponent >();
		t->transform.set_identity();
		t->transform.set_position( p + Vector( 0.f, 2.f, 0.f ) );
		e->add_component< RenderComponent >();
		auto* m = e->add_component< ModelComponent >();
		m->model = box_model; // 見た目は 1x1x1（当たりは下記 Box）

		auto* rb = e->add_component< RigidBodyComponent >();
		rb->shape_type = RigidBodyShapeType::Box;
		rb->shape_size = Vector( 2.f, 3.f, 2.f );
		rb->offset = Vector( 0.f, 2.f, 0.f );
		rb->mass = 0.f;
	}

	// --- 敵（ウェイポイント巡回） ---
	spawn_enemy(
		Vector( 0.f, 0.5f, 6.f ),
		{ Vector( -8.f, 0.5f, 6.f ), Vector( 8.f, 0.5f, 6.f ) } );

	spawn_enemy(
		Vector( 6.f, 0.5f, 16.f ),
		{ Vector( 6.f, 0.5f, 16.f ), Vector( 6.f, 0.5f, 2.f ), Vector( -6.f, 0.5f, 2.f ) } );
}

void EnemyAiTestScene::spawn_enemy( const Vector& start, const std::vector< Vector >& waypoints )
{
	auto* e = get_entity_manager()->create_entity();

	auto* t = e->add_component< TransformComponent >();
	t->transform.set_identity();
	t->transform.set_position( start );

	// 見た目（プレースホルダの箱＝ロボット）
	e->add_component< RenderComponent >();
	auto* m = e->add_component< ModelComponent >();
	m->model = get_graphics_manager()->get_model_handle( get_graphics_manager()->load_model( "box-1x1x1" ) );

	auto* perception = e->add_component< blue_sky::ecs::PerceptionComponent >();
	perception->eye_height = 1.0f;
	perception->cone.view_distance      = 18.f;
	perception->cone.horizontal_half_fov = 1.0472f; // 60deg
	perception->cone.vertical_up_limit  = 0.3491f;  // 20deg

	e->add_component< blue_sky::ecs::AlertComponent >();

	auto* patrol = e->add_component< blue_sky::ecs::PatrolComponent >();
	patrol->waypoints = waypoints;
	patrol->move_speed = 2.5f;

	enemies_.push_back( e );
}

void EnemyAiTestScene::update()
{
	Scene::update();

	get_graphics_manager()->update();
	get_graphics_manager()->clear_debug_bullet();

	get_entity_manager()->update();

	// clear の後・render_debug_bullet の前にコーン等を積む
	draw_debug();
}

void EnemyAiTestScene::draw_debug()
{
	auto* dd = get_bullet_debug_draw();
	if ( ! dd )
	{
		return;
	}

	for ( auto* e : enemies_ )
	{
		auto* transform  = e->get_component< TransformComponent >();
		auto* perception = e->get_component< blue_sky::ecs::PerceptionComponent >();
		auto* alert      = e->get_component< blue_sky::ecs::AlertComponent >();
		auto* patrol     = e->get_component< blue_sky::ecs::PatrolComponent >();
		if ( ! transform || ! perception || ! alert )
		{
			continue;
		}

		const Vector pos     = transform->transform.get_position();
		const Vector forward = transform->transform.forward();
		const core::Vector col = state_color( alert->state );

		// 視界錐は「床に投影した扇」として描く。
		// 目の高さの水平な扇だと、同じ目線高さの FPS 視点からは真横＝1本の線に見えて
		// 三角形が判別できないため、地面すれすれに描いて見下ろす形で扇が見えるようにする。
		const float floor_y = 0.05f; // 地面のわずか上（z-fighting 回避）
		const Vector apex( pos.x(), floor_y, pos.z() );
		const float  base = std::atan2( forward.x(), forward.z() );
		const float  half = perception->cone.horizontal_half_fov;
		const float  dist = perception->cone.view_distance;
		const int    N    = 12;
		Vector prev_end;
		for ( int i = 0; i <= N; ++i )
		{
			const float a = base - half + ( 2.f * half ) * static_cast< float >( i ) / N;
			const Vector dir( std::sin( a ), 0.f, std::cos( a ) );
			const Vector end = apex + dir * dist;

			dd->drawLine( apex, end, col );         // 稜線（床）
			if ( i > 0 )
			{
				dd->drawLine( prev_end, end, col ); // 弧（床）
			}
			prev_end = end;
		}
		// 敵の体と扇をつなぐ縦線（足元→目の高さ）。錐の原点が敵だと分かるように。
		dd->drawLine( apex, pos + Vector( 0.f, perception->eye_height, 0.f ), col );

		// パトロール経路（薄い青の折れ線・床）
		if ( patrol && patrol->waypoints.size() >= 2 )
		{
			const core::Vector path_col( 0.2f, 0.4f, 1.f );
			for ( std::size_t i = 0; i < patrol->waypoints.size(); ++i )
			{
				Vector a = patrol->waypoints[ i ];
				Vector b = patrol->waypoints[ ( i + 1 ) % patrol->waypoints.size() ];
				a.set_y( floor_y );
				b.set_y( floor_y );
				dd->drawLine( a, b, path_col );
			}
		}

		// 最終視認地点（警戒中のみ、状態色の縦十字）
		if ( perception->has_last_seen && alert->state != ai::AlertState::Unaware )
		{
			const Vector p = perception->last_seen_position;
			dd->drawLine( p - Vector( 0.5f, 0.f, 0.f ), p + Vector( 0.5f, 0.f, 0.f ), col );
			dd->drawLine( p - Vector( 0.f, 0.f, 0.5f ), p + Vector( 0.f, 0.f, 0.5f ), col );
			dd->drawLine( p, p + Vector( 0.f, 2.f, 0.f ), col );
		}
	}
}

void EnemyAiTestScene::render()
{
	// 描く System（RenderSystem など）を、画面のクリアの後に動かす
	get_entity_manager()->render();

	get_graphics_manager()->render_debug_bullet();

	std::stringstream ss;
	ss << "Enemy AI Test  FPS: " << get_main_loop()->get_last_fps() << "\n";
	ss << "WASD move, mouse look. Enter enemy cone to raise alert.\n";

	for ( std::size_t i = 0; i < enemies_.size(); ++i )
	{
		auto* alert = enemies_[ i ]->get_component< blue_sky::ecs::AlertComponent >();
		if ( ! alert ) { continue; }
		ss << "enemy[" << i << "] " << state_name( alert->state )
		   << "  awareness=" << alert->awareness << "\n";
	}

	get_graphics_manager()->draw_text( 10.f, 10.f, 480.f, 200.f, ss.str().c_str(), Color::White );
}

} // namespace blue_sky
