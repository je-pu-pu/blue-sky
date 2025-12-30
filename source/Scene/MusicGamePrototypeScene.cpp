#include "MusicGamePrototypeScene.h"

#include <blue_sky/SceneManager.h>
#include <blue_sky/GameMain.h>
#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/Input.h>

#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/component/CameraComponent.h>
#include <core/ecs/component/ModelComponent.h>
#include <core/ecs/component/RigidBodyComponent.h>
#include <core/ecs/component/RenderComponent.h>
#include <core/ecs/component/FpsPlayerComponent.h>
#include <core/ecs/component/BalloonComponent.h>

#include <core/ecs/system/RenderSystem.h>
#include <core/ecs/system/PhysicsSystem.h>
#include <core/ecs/system/FpsPlayerControlSystem.h>
#include <core/ecs/system/BalloonSystem.h>

#include <core/ecs/EntityManager.h>
#include <core/Service.h>
#include <core/physics/PhysicsManager.h>
#include <core/graphics/Direct3D11/BulletDebugDraw.h>

#include <core/sound/SoundManager.h>
#include <core/sound/SoundEngine.h>
#include <core/sound/MidiSequencer.h>

#include <common/random.h>

#include <game/MainLoop.h>

#include <algorithm>
#include <vector>
#include <sstream>

namespace blue_sky
{

using RigidBodyShapeType = core::physics::RigidBodyShapeType;

MusicGamePrototypeScene::MusicGamePrototypeScene()
	: player_( EntityManager::get_instance()->create_entity() )
{
	get_graphics_manager()->setup_default_shaders();
	get_graphics_manager()->load_paper_textures();

	// PhysicsManager にデバッグ描画を設定
	core::get_physics_manager()->set_debug_drawer( GameMain::get_instance()->get_bullet_debug_draw() );

	// System を追加する (順序が重要)
	get_entity_manager()->add_system< core::ecs::PhysicsSystem >();
	get_entity_manager()->add_system< core::ecs::FpsPlayerControlSystem >( 10 );
	get_entity_manager()->add_system< core::ecs::BalloonSystem >( 20 );
	get_entity_manager()->add_system< core::ecs::RenderSystem >( 1000 );

	// プレイヤーを設定 (ビルの屋上に配置)
	player_transform_ = player_->add_component< core::ecs::TransformComponent >();
	player_transform_->transform.set_identity();
	player_transform_->transform.set_position( Vector( 0.f, 102.f, 0.f ) );

	// FPS プレイヤーコンポーネント
	fps_player_ = player_->add_component< core::ecs::FpsPlayerComponent >();
	fps_player_->move_speed = 5.f;
	fps_player_->eye_height = 0.8f;
	fps_player_->jump_impulse = 350.f; // 70kg × 5m/s = 350 N·s

	// RigidBody (Capsule 形状)
	auto* rigid_body = player_->add_component< core::ecs::RigidBodyComponent >();
	rigid_body->shape_type = RigidBodyShapeType::Capsule;
	rigid_body->shape_size = Vector( 0.3f, 1.0f, 0.f ); // radius=0.3, height=1.0
	rigid_body->offset = Vector( 0.f, 0.8f, 0.f );       // 足元を原点に
	rigid_body->mass = 70.f;                              // 70kg
	rigid_body->angular_factor = Vector( 0.f, 0.f, 0.f ); // 転倒防止
	rigid_body->friction = 0.1f;
	rigid_body->collision_group = core::physics::CollisionGroup::Player;
	rigid_body->collision_mask = core::physics::CollisionGroup::All;

	// カメラコンポーネント
	player_camera_ = player_->add_component< core::ecs::CameraComponent >();
	player_camera_->eye_offset = Vector( 0.f, 1.5f, 0.f ); // 目の高さ (足元から 1.5m)

	get_graphics_manager()->set_main_camera_info( player_transform_, player_camera_ );

	// 地面を配置
	{
		auto* e = get_entity_manager()->create_entity();
		auto* t = e->add_component< core::ecs::TransformComponent >();
		t->transform.set_identity();
		e->add_component< core::ecs::RenderComponent >();
		auto* m = e->add_component< core::ecs::ModelComponent>();
		m->model = get_graphics_manager()->load_model( "ground" );

		// 地面の RigidBody (静的)
		auto* ground_rigid_body = e->add_component< core::ecs::RigidBodyComponent >();
		ground_rigid_body->shape_type = RigidBodyShapeType::Box;
		ground_rigid_body->shape_size = Vector( 500.f, 1.f, 500.f ); // 大きな平面
		ground_rigid_body->offset = Vector( 0.f, -1.f, 0.f );        // 地表が Y=0
		ground_rigid_body->mass = 0.f;
	}

	Model* models[ 2 ] = {
		get_graphics_manager()->load_model( "building-red" ),
		get_graphics_manager()->load_model( "building-white" ),
	};

	/// @todo もっとスマートにやる
	// シェーダーを設定 ( flat から lit に変更 )
	for ( auto* m : models )
	{
		for ( size_t n = 0; n < m->get_shader_count(); n++ )
		{
			auto* shader = get_graphics_manager()->get_shader( "lit" )->clone();
			shader->set_texture_at( 0, m->get_shader_at( n )->get_texture_at( 0 ) );

			m->set_shader_at( n, shader );
		}
	}

	// 建物をランダムに配置 (屋上の位置を記録)
	struct BuildingInfo { float x, y, z, rooftop_y; bool exists; };
	std::vector< std::vector< BuildingInfo > > buildings( 30, std::vector< BuildingInfo >( 10 ) );

	for ( int z = 0; z < 30; z++ )
	{
		for ( int x = 0; x < 10; x++ )
		{
			// (0, 0) はプレイヤーの初期位置なので確実に配置
			const bool is_spawn_point = ( x == 0 && z == 0 );

			if ( ! is_spawn_point && common::random( 0, 1 ) == 0 )
			{
				buildings[ z ][ x ].exists = false;
				continue;
			}

			auto* e = get_entity_manager()->create_entity();
			auto* t = e->add_component< core::ecs::TransformComponent >();
			t->transform.set_identity();

			// スポーン地点のビルは最大高さ (Y=0) に固定
			float building_y = is_spawn_point ? 0.f : common::random( -8, 0 ) * 10.f;
			float world_x = x * 12.f;
			float world_z = z * 12.f;
			t->transform.set_position( Vector( world_x, building_y, world_z ) );

			// ビル情報を記録 (屋上 = building_y + 100)
			buildings[ z ][ x ] = { world_x, building_y, world_z, building_y + 100.f, true };

			e->add_component< core::ecs::RenderComponent >();
			auto* m = e->add_component< core::ecs::ModelComponent>();
			m->model = models[ common::random( 0, 1 ) ];

			auto* building_rigid_body = e->add_component< core::ecs::RigidBodyComponent >();
			building_rigid_body->shape_type = RigidBodyShapeType::Box;
			building_rigid_body->shape_size = Vector( 5.f, 50.f, 5.f );
			building_rigid_body->offset = Vector( 0.f, 50.f, 0.f );
			building_rigid_body->mass = 0.f; // 静的オブジェクト
		}
	}

	// 風船モデルを読み込み
	auto* balloon_model = get_graphics_manager()->load_model( "balloon" );

	// 隣接するビル間に風船の連なりを配置
	for ( int z = 0; z < 30; z++ )
	{
		for ( int x = 0; x < 10; x++ )
		{
			if ( ! buildings[ z ][ x ].exists ) continue;

			const auto& b1 = buildings[ z ][ x ];

			// X方向の隣のビルと接続
			if ( x + 1 < 10 && buildings[ z ][ x + 1 ].exists )
			{
				const auto& b2 = buildings[ z ][ x + 1 ];

				// 2つのビルの屋上の中間点を計算
				float avg_y = ( b1.rooftop_y + b2.rooftop_y ) / 2.f + 5.f; // 少し上に

				// 風船を3つ配置
				for ( int i = 1; i <= 3; i++ )
				{
					float t = static_cast< float >( i ) / 4.f;
					float bx = b1.x + ( b2.x - b1.x ) * t;
					float bz = b1.z + ( b2.z - b1.z ) * t;

					auto* balloon_entity = get_entity_manager()->create_entity();
					auto* bt = balloon_entity->add_component< core::ecs::TransformComponent >();
					bt->transform.set_identity();
					bt->transform.set_position( Vector( bx, avg_y, bz ) );

					balloon_entity->add_component< core::ecs::RenderComponent >();
					auto* bm = balloon_entity->add_component< core::ecs::ModelComponent >();
					bm->model = balloon_model;

					auto* bc = balloon_entity->add_component< core::ecs::BalloonComponent >();
					bc->lift_force = 400.f;

					// 風船の RigidBody (静的な球)
					auto* balloon_rb = balloon_entity->add_component< core::ecs::RigidBodyComponent >();
					balloon_rb->shape_type = RigidBodyShapeType::Sphere;
					balloon_rb->shape_size = Vector( 1.5f, 0.f, 0.f ); // 半径 1.5m
					balloon_rb->mass = 0.f; // 静的オブジェクト
					balloon_rb->collision_group = core::physics::CollisionGroup::Balloon;
					balloon_rb->collision_mask = core::physics::CollisionGroup::Player; // プレイヤーとのみ衝突
				}
			}

			// Z方向の隣のビルと接続
			if ( z + 1 < 30 && buildings[ z + 1 ][ x ].exists )
			{
				const auto& b2 = buildings[ z + 1 ][ x ];

				float avg_y = ( b1.rooftop_y + b2.rooftop_y ) / 2.f + 5.f;

				for ( int i = 1; i <= 3; i++ )
				{
					float t = static_cast< float >( i ) / 4.f;
					float bx = b1.x + ( b2.x - b1.x ) * t;
					float bz = b1.z + ( b2.z - b1.z ) * t;

					auto* balloon_entity = get_entity_manager()->create_entity();
					auto* bt = balloon_entity->add_component< core::ecs::TransformComponent >();
					bt->transform.set_identity();
					bt->transform.set_position( Vector( bx, avg_y, bz ) );

					balloon_entity->add_component< core::ecs::RenderComponent >();
					auto* bm = balloon_entity->add_component< core::ecs::ModelComponent >();
					bm->model = balloon_model;

					auto* bc = balloon_entity->add_component< core::ecs::BalloonComponent >();
					bc->lift_force = 400.f;

					// 風船の RigidBody (静的な球)
					auto* balloon_rb = balloon_entity->add_component< core::ecs::RigidBodyComponent >();
					balloon_rb->shape_type = RigidBodyShapeType::Sphere;
					balloon_rb->shape_size = Vector( 1.5f, 0.f, 0.f ); // 半径 1.5m
					balloon_rb->mass = 0.f; // 静的オブジェクト
					balloon_rb->collision_group = core::physics::CollisionGroup::Balloon;
					balloon_rb->collision_mask = core::physics::CollisionGroup::Player; // プレイヤーとのみ衝突
				}
			}
		}
	}

	// MIDI シーケンサーを初期化
	midi_sequencer_ = std::make_unique< core::sound::MidiSequencer >(
		"media/music/gun.mid",
		get_sound_manager()->get_sound_engine()->get_midi_synthesizer()
	);
}

void MusicGamePrototypeScene::update()
{
	Scene::update();

	if ( get_input()->push( Input::Button::A ) )
	{
		// 落下する球を作成
		auto* e = get_entity_manager()->create_entity();
		auto* t = e->add_component< core::ecs::TransformComponent >();
		t->transform.set_identity();
		t->transform.set_position( player_transform_->transform.get_position() + Vector( 0.f, 0.5f, 0.f ) );

		e->add_component< core::ecs::RenderComponent >();
		auto* m = e->add_component< core::ecs::ModelComponent >();
		m->model = get_graphics_manager()->load_model( "box-1x1x1" );

		auto* rigid_body = e->add_component< core::ecs::RigidBodyComponent >();
		rigid_body->shape_type = RigidBodyShapeType::Box;
		rigid_body->shape_size = Vector( 0.5f, 0.5f, 0.5f );
		rigid_body->offset = Vector( 0.f, 0.5f, 0.f );
		rigid_body->mass = 1.f; // 動的オブジェクト
	}

	get_graphics_manager()->update();
	get_graphics_manager()->clear_debug_bullet();

	get_entity_manager()->update();

	// MIDI シーケンサーを処理
	midi_sequencer_->process();

	// ビート進行度を計算 (1.0 → 0.0)
	// get_ticks() は現在のビート内での経過 ticks を返す
	float beat_progress = 0.f;
	const float ticks_per_beat = midi_sequencer_->get_ticks_per_beat();
	if ( ticks_per_beat > 0.f )
	{
		beat_progress = 1.f - static_cast< float >( midi_sequencer_->get_ticks() ) / ticks_per_beat;
		beat_progress = std::clamp( beat_progress, 0.f, 1.f );
	}
	get_graphics_manager()->get_frame_render_data()->data().beat_progress = beat_progress;
}

void MusicGamePrototypeScene::render()
{
	// デバッグ描画を行う
	get_graphics_manager()->render_debug_bullet();

	// FPS / ドローコール数 表示
	std::stringstream ss;
	ss << "FPS: " << get_game_main()->get_main_loop()->get_last_fps() << "\n";
	ss << "Draw: " << get_graphics_manager()->get_draw_count();

	get_graphics_manager()->draw_text( 10.f, 10.f, 200.f, 150.f, ss.str().c_str(), Color::White );
}

} // namespace blue_sky
