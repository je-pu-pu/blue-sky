#include "ParticleSystemTestScene.h"

#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/ActiveObjectPhysics.h>
#include <blue_sky/Input.h>

#include <core/ecs/Component/CameraComponent.h>
#include <core/ecs/Component/TransformComponent.h>
#include <blue_sky/ecs/component/TransformControlComponent.h>
#include <core/ecs/Component/ParticleSystemComponent.h>
#include <blue_sky/ecs/system/TransformControlSystem.h>
#include <core/ecs/System/RenderSystem.h>
#include <core/ecs/System/ParticleSystem.h>
#include <core/ecs/System/ParticleRenderSystem.h>
#include <core/ecs/EntityManager.h>

#include <game/MainLoop.h>

#include <sstream>

namespace blue_sky
{

ParticleSystemTestScene::ParticleSystemTestScene()
	: camera_( EntityManager::get_instance()->create_entity() )
{
	// Physics
	get_active_object_physics()->add_ground_rigid_body( Vector( 1000, 1, 1000 ) );

	get_graphics_manager()->setup_default_shaders();
	get_graphics_manager()->load_paper_textures();

	// System を追加する
	get_entity_manager()->add_system< core::ecs::ParticleRenderSystem >( 1001 );
	get_entity_manager()->add_system< core::ecs::ParticleSystem >( 0 );
	get_entity_manager()->add_system< blue_sky::ecs::TransformControlSystem >();
	get_entity_manager()->add_system< core::ecs::RenderSystem >( 1000 );

	// Entity と Component を追加する
	auto particle_system = get_entity_manager()->create_entity();
	particle_system->add_component< core::ecs::TransformComponent >()->transform.set_identity();
	particle_system->add_component< core::ecs::ParticleSystemComponent >();

	auto camera_transform_component = camera_->add_component< core::ecs::TransformComponent >();
	camera_transform_component->transform.set_identity();
	camera_transform_component->transform.set_position( Vector( 0.f, 1.5f, -10.f ) );

	camera_->add_component< blue_sky::ecs::TransformControlComponent >();
	auto camera_component = camera_->add_component< core::ecs::CameraComponent >();

	get_graphics_manager()->set_main_camera_info( camera_transform_component, camera_component );
}

ParticleSystemTestScene::~ParticleSystemTestScene()
{

}

void ParticleSystemTestScene::update()
{
	Scene::update();

	get_entity_manager()->update();
	get_graphics_manager()->update();

	get_graphics_manager()->clear_debug_bullet();
	get_active_object_physics()->update( get_elapsed_time() );
}

void ParticleSystemTestScene::render()
{
	std::stringstream ss;
	ss << "Time : " << get_total_elapsed_time() << '\n';
	ss << "FPS : " << get_main_loop()->get_last_fps() << '\n';
	ss << "pass count : " << get_graphics_manager()->get_pass_count() << '\n';
	ss << "draw count : " << get_graphics_manager()->get_draw_count() << '\n';

	get_graphics_manager()->draw_text( 10.f, 10.f, get_width() - 10.f, get_height() - 10.f, ss.str().c_str(), Color::White );
}

} // namespace blue_sky
