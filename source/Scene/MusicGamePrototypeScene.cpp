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

#include <core/ecs/system/RenderSystem.h>
#include <core/ecs/system/PhysicsSystem.h>
#include <core/ecs/system/FpsPlayerControlSystem.h>

#include <core/ecs/EntityManager.h>
#include <core/physics/PhysicsManager.h>
#include <core/graphics/Direct3D11/BulletDebugDraw.h>

#include <common/random.h>

namespace blue_sky
{

using RigidBodyShapeType = core::physics::RigidBodyShapeType;

MusicGamePrototypeScene::MusicGamePrototypeScene()
	: player_( EntityManager::get_instance()->create_entity() )
{
	get_graphics_manager()->setup_default_shaders();
	get_graphics_manager()->load_paper_textures();

	// PhysicsManager にデバッグ描画を設定
	core::physics::PhysicsManager::get_instance()->set_debug_drawer( GameMain::get_instance()->get_bullet_debug_draw() );

	// System を追加する (順序が重要)
	get_entity_manager()->add_system< core::ecs::PhysicsSystem >();
	get_entity_manager()->add_system< core::ecs::FpsPlayerControlSystem >( 10 );
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

	// 建物をランダムに配置
	for ( int z = 0; z < 30; z++ )
	{
		for ( int x = 0; x < 10; x++ )
		{
			// (0, 0) はプレイヤーの初期位置なので確実に配置
			const bool is_spawn_point = ( x == 0 && z == 0 );

			if ( ! is_spawn_point && common::random( 0, 1 ) == 0 )
			{
				continue;
			}

			auto* e = get_entity_manager()->create_entity();
			auto* t = e->add_component< core::ecs::TransformComponent >();
			t->transform.set_identity();

			// スポーン地点のビルは最大高さ (Y=0) に固定
			float building_y = is_spawn_point ? 0.f : common::random( -8, 0 ) * 10.f;
			t->transform.set_position( Vector( x * 12.f, building_y, z * 12.f ) );

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
}

void MusicGamePrototypeScene::render()
{
	// デバッグ描画を行う
	get_graphics_manager()->render_debug_bullet();
}

} // namespace blue_sky
