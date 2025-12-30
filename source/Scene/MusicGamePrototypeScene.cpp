#include "MusicGamePrototypeScene.h"

#include <blue_sky/SceneManager.h>
#include <blue_sky/GameMain.h>
#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/Input.h>

#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/component/TransformControlComponent.h>
#include <core/ecs/component/CameraComponent.h>
#include <core/ecs/component/ModelComponent.h>
#include <core/ecs/component/RigidBodyComponent.h>
#include <core/ecs/component/RenderComponent.h>

#include <core/ecs/system/RenderSystem.h>
#include <core/ecs/system/TransformControlSystem.h>
#include <core/ecs/system/PhysicsSystem.h>

#include <core/ecs/EntityManager.h>
#include <core/physics/PhysicsManager.h>
#include <core/graphics/Direct3D11/BulletDebugDraw.h>

#include <common/random.h>

namespace blue_sky
{

using RigidBodyShapeType = core::physics::RigidBodyShapeType;

MusicGamePrototypeScene::MusicGamePrototypeScene()
	: camera_( EntityManager::get_instance()->create_entity() )
{
	get_graphics_manager()->setup_default_shaders();
	get_graphics_manager()->load_paper_textures();

	// PhysicsManager にデバッグ描画を設定
	core::physics::PhysicsManager::get_instance()->set_debug_drawer( GameMain::get_instance()->get_bullet_debug_draw() );

	// System を追加する
	get_entity_manager()->add_system< core::ecs::TransformControlSystem >();
	get_entity_manager()->add_system< core::ecs::PhysicsSystem >();
	get_entity_manager()->add_system< core::ecs::RenderSystem >( 1000 );

	// カメラを設定
	camera_transform_ = camera_->add_component< core::ecs::TransformComponent >();
	camera_transform_->transform.set_identity();
	camera_transform_->transform.set_position( Vector( 0.f, 3.f, -10.f ) );

	camera_->add_component< core::ecs::TransformControlComponent >();
	camera_component_ = camera_->add_component< core::ecs::CameraComponent >();

	get_graphics_manager()->set_main_camera_info( camera_transform_, camera_component_ );

	// 地面を配置
	{
		auto* e = get_entity_manager()->create_entity();
		auto* t = e->add_component< core::ecs::TransformComponent >();
		t->transform.set_identity();
		e->add_component< core::ecs::RenderComponent >();
		auto* m = e->add_component< core::ecs::ModelComponent>();
		m->model = get_graphics_manager()->load_model( "ground" );
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
			if ( common::random( 0, 1 ) == 0 )
			{
				continue;
			}

			auto* e = get_entity_manager()->create_entity();
			auto* t = e->add_component< core::ecs::TransformComponent >();
			t->transform.set_identity();
			t->transform.set_position( Vector( x * 12.f, common::random( -8, 0 ) * 10.f, z * 12.f ) );

			e->add_component< core::ecs::RenderComponent >();
			auto* m = e->add_component< core::ecs::ModelComponent>();
			m->model = models[ common::random( 0, 1 ) ];

			auto* rigid_body = e->add_component< core::ecs::RigidBodyComponent >();
			rigid_body->shape_type = RigidBodyShapeType::Box;
			rigid_body->shape_size = Vector( 5.f, 50.f, 5.f );
			rigid_body->offset = Vector( 0.f, 50.f, 0.f );
			rigid_body->mass = 0.f; // 静的オブジェクト
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
		t->transform.set_position( camera_transform_->transform.get_position() + Vector( 0.f, 0.5f, 0.f ) );

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
