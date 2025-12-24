#include "PhysicsTestScene.h"

#include <blue_sky/SceneManager.h>
#include <blue_sky/GameMain.h>
#include <blue_sky/graphics/GraphicsManager.h>

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

namespace blue_sky
{

using RigidBodyShapeType = core::physics::RigidBodyShapeType;

PhysicsTestScene::PhysicsTestScene()
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

	// 地面を作成 (静的な Box、質量 0)
	auto* ground = get_entity_manager()->create_entity();
	{
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

	// 落下するボックスを作成
	current_entity_ = get_entity_manager()->create_entity();
	{
		current_entity_transform_ = current_entity_->add_component< core::ecs::TransformComponent >();
		current_entity_transform_->transform.set_identity();
		current_entity_transform_->transform.set_position( Vector( 0.f, 10.f, 0.f ) );

		current_entity_->add_component< core::ecs::RenderComponent >();
		current_entity_model_ = current_entity_->add_component< core::ecs::ModelComponent >();
		current_entity_model_->model = get_graphics_manager()->load_model( "goal" );

		auto* physics_component = current_entity_->add_component< core::ecs::RigidBodyComponent >();
		physics_component->shape_type = RigidBodyShapeType::Box;
		physics_component->shape_size = Vector( 0.5f, 0.5f, 0.5f );
		physics_component->mass = 1.f; // 動的オブジェクト
	}

	// カメラを設定
	camera_transform_ = camera_->add_component< core::ecs::TransformComponent >();
	camera_transform_->transform.set_identity();
	camera_transform_->transform.set_position( Vector( 0.f, 3.f, -10.f ) );

	camera_->add_component< core::ecs::TransformControlComponent >();
	camera_component_ = camera_->add_component< core::ecs::CameraComponent >();

	get_graphics_manager()->set_main_camera_info( camera_transform_, camera_component_ );
}

void PhysicsTestScene::update()
{
	Scene::update();

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
