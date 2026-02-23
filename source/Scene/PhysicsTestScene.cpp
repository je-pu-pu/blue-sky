#include "PhysicsTestScene.h"

#include <blue_sky/SceneManager.h>
#include <blue_sky/GameMain.h>
#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/Input.h>

#include <core/ecs/component/TransformComponent.h>
#include <blue_sky/ecs/component/TransformControlComponent.h>
#include <core/ecs/component/CameraComponent.h>
#include <core/ecs/component/ModelComponent.h>
#include <core/ecs/component/RigidBodyComponent.h>
#include <core/ecs/component/RenderComponent.h>

#include <core/ecs/system/RenderSystem.h>
#include <blue_sky/ecs/system/TransformControlSystem.h>
#include <core/ecs/system/PhysicsSystem.h>

#include <core/ecs/EntityManager.h>
#include <core/Service.h>
#include <core/physics/PhysicsManager.h>
#include <core/graphics/Direct3D11/BulletDebugDraw.h>

#include <common/random.h>

namespace blue_sky
{

using RigidBodyShapeType = core::physics::RigidBodyShapeType;

core::graphics::Model* models[] = { nullptr, nullptr, nullptr };

PhysicsTestScene::PhysicsTestScene()
	: camera_( EntityManager::get_instance()->create_entity() )
{
	get_graphics_manager()->setup_default_shaders();
	get_graphics_manager()->load_paper_textures();

	// PhysicsManager にデバッグ描画を設定
	core::get_physics_manager()->set_debug_drawer( GameMain::get_instance()->get_bullet_debug_draw() );

	// System を追加する
	get_entity_manager()->add_system< blue_sky::ecs::TransformControlSystem >();
	get_entity_manager()->add_system< core::ecs::PhysicsSystem >();
	get_entity_manager()->add_system< core::ecs::RenderSystem >( 1000 );

	// 地面を作成 (静的な Box、質量 0)
	{
		auto* ground = get_entity_manager()->create_entity();
		auto* ground_transform = ground->add_component< core::ecs::TransformComponent >();
		ground_transform->transform.set_identity();
		ground_transform->transform.set_position( Vector( 0.f, 0.f, 0.f ) );

		auto* ground_rigid_body = ground->add_component< core::ecs::RigidBodyComponent >();
		ground_rigid_body->shape_type = RigidBodyShapeType::Box;
		ground_rigid_body->shape_size = Vector( 100.f, 0.1f, 100.f );
		ground_rigid_body->offset = Vector( 0.f, -0.1f, 0.f );
		ground_rigid_body->mass = 0.f; // 静的オブジェクト
	}

	// 壁を作成 (静的な Box、質量 0)
	{
		auto* ground = get_entity_manager()->create_entity();
		auto* ground_transform = ground->add_component< core::ecs::TransformComponent >();
		ground_transform->transform.set_identity();
		ground_transform->transform.set_position( Vector( 0.f, 0.f, 0.f ) );

		ground->add_component< core::ecs::RenderComponent >();
		auto* ground_model = ground->add_component< core::ecs::ModelComponent >();
		ground_model->model = get_graphics_manager()->load_model( "wall-1" );

		auto* ground_rigid_body = ground->add_component< core::ecs::RigidBodyComponent >();
		ground_rigid_body->shape_type = RigidBodyShapeType::Box;
		ground_rigid_body->shape_size = Vector( 2.f, 0.875f, 0.05f );
		ground_rigid_body->offset = Vector( 0.f, 0.875f, 0.f );
		ground_rigid_body->mass = 0.f; // 静的オブジェクト
	}

	// カメラを設定
	camera_transform_ = camera_->add_component< core::ecs::TransformComponent >();
	camera_transform_->transform.set_identity();
	camera_transform_->transform.set_position( Vector( 0.f, 3.f, -10.f ) );

	camera_->add_component< blue_sky::ecs::TransformControlComponent >();
	camera_component_ = camera_->add_component< core::ecs::CameraComponent >();

	get_graphics_manager()->set_main_camera_info( camera_transform_, camera_component_ );


	GraphicsManager::Shader* shaders[] = {
			get_graphics_manager()->clone_shader( get_graphics_manager()->get_shader( "matcap" ) ),
			get_graphics_manager()->clone_shader( get_graphics_manager()->get_shader( "matcap" ) ),
			get_graphics_manager()->clone_shader( get_graphics_manager()->get_shader( "matcap" ) ),
		};

	shaders[ 0 ]->set_texture_at( 0, get_graphics_manager()->load_named_texture( "matcap-a", "media/texture/matcap/test/mc20.jpg" ) );
	shaders[ 1 ]->set_texture_at( 0, get_graphics_manager()->load_named_texture( "matcap-b", "media/texture/matcap/test/mc13.jpg" ) );
	shaders[ 2 ]->set_texture_at( 0, get_graphics_manager()->load_named_texture( "matcap-c", "media/texture/matcap/test/mc11.jpg" ) );

	models[ 0 ] = get_graphics_manager()->clone_model( get_graphics_manager()->load_model( "sphere" ) );
	models[ 1 ] = get_graphics_manager()->clone_model( get_graphics_manager()->load_model( "sphere" ) );
	models[ 2 ] = get_graphics_manager()->clone_model( get_graphics_manager()->load_model( "sphere" ) );

	models[ 0 ]->set_shader_at( 0, shaders[ 0 ] );
	models[ 1 ]->set_shader_at( 0, shaders[ 1 ] );
	models[ 2 ]->set_shader_at( 0, shaders[ 2 ] );
}

void PhysicsTestScene::update()
{
	Scene::update();

	if ( get_input()->push( Input::Button::A ) )
	{
		// 落下するボックスを作成
		auto* e = get_entity_manager()->create_entity();
		auto* t = e->add_component< core::ecs::TransformComponent >();
		t->transform.set_identity();
		t->transform.set_position( Vector( common::random( -0.01f, 0.01f ), 10.f, common::random( -0.01f, 0.01f ) ) );

		e->add_component< core::ecs::RenderComponent >();
		auto* m = e->add_component< core::ecs::ModelComponent >();
		m->model = models[ common::random( 0, 2 ) ];

		auto* physics_component = e->add_component< core::ecs::RigidBodyComponent >();
		physics_component->shape_type = RigidBodyShapeType::Sphere;
		physics_component->shape_size = Vector( 1.f, 1.f, 1.f );
		physics_component->mass = 1.f; // 動的オブジェクト
	}

	get_graphics_manager()->update();
	get_graphics_manager()->clear_debug_bullet();

	get_entity_manager()->update();
}

void PhysicsTestScene::render()
{
	// デバッグ描画を行う
	get_graphics_manager()->render_debug_bullet();
}

} // namespace blue_sky
