#include "PhysicsTestScene.h"

#include <blue_sky/SceneManager.h>
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

namespace blue_sky
{

PhysicsTestScene::PhysicsTestScene()
	: camera_( EntityManager::get_instance()->create_entity() )
{
	get_graphics_manager()->setup_default_shaders();
	get_graphics_manager()->load_paper_textures();

	// System ‚ð’Ç‰Á‚·‚é
	get_entity_manager()->add_system< core::ecs::TransformControlSystem >();
	get_entity_manager()->add_system< core::ecs::PhysicsSystem >();
	get_entity_manager()->add_system< core::ecs::RenderSystem >( 1000 );

	// Entity ‚Æ Component ‚ð’Ç‰Á‚·‚é
	current_entity_ = get_entity_manager()->create_entity();
	current_entity_transform_ = current_entity_->add_component< core::ecs::TransformComponent >();
	current_entity_transform_->transform.set_identity();
	current_entity_->add_component< core::ecs::RenderComponent >();
	current_entity_model_ = current_entity_->add_component< core::ecs::ModelComponent >();

	current_entity_model_->model = get_graphics_manager()->load_model( "goal" );

	// auto physics_component = current_entity_->add_component< core::ecs::RigidBodyComponent >();

	camera_transform_ = camera_->add_component< core::ecs::TransformComponent >();
	camera_transform_->transform.set_identity();
	camera_transform_->transform.set_position( Vector( 0.f, 1.5f, -10.f ) );

	camera_->add_component< core::ecs::TransformControlComponent >();
	camera_component_ = camera_->add_component< core::ecs::CameraComponent >();

	get_graphics_manager()->set_main_camera_info( camera_transform_, camera_component_ );
}

void PhysicsTestScene::update()
{
	Scene::update();

	/*
	if ( get_input()->is_mouse_button_down( core::input::MouseButton::Left ) )
	{
		POINT mouse_pos;
		get_input()->get_mouse_position( & mouse_pos.x, & mouse_pos.y );
		Vector near_point = get_graphics_manager()->unproject_to_world( static_cast< float_t >( mouse_pos.x ), static_cast< float_t >( mouse_pos.y ), 0.f );
		Vector far_point = get_graphics_manager()->unproject_to_world( static_cast< float_t >( mouse_pos.x ), static_cast< float_t >( mouse_pos.y ), 1.f );
		Vector dir = ( far_point - near_point ).get_normalized();
		std::cout << "near : " << near_point << ", far : " << far_point << ", dir : " << dir << std::endl;
		get_physics_manager()->shoot_ray( near_point, dir * 1000.f );
	}
	*/

	get_graphics_manager()->update();
	get_graphics_manager()->clear_debug_bullet();

	get_entity_manager()->update();
}

void PhysicsTestScene::render()
{
	// 
}

} // namespace blue_sky
