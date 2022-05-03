#include "TransformTestScene.h"

#include <blue_sky/graphics/GraphicsManager.h>

#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/component/TransformControlComponent.h>
#include <core/ecs/component/CameraComponent.h>
#include <core/ecs/component/RenderComponent.h>

#include <core/ecs/system/RenderSystem.h>
#include <core/ecs/system/TransformControlSystem.h>
#include <core/ecs/EntityManager.h>

#include <game/MainLoop.h>

#include <common/math.h>

#include <imgui.h>

#include <sstream>

namespace blue_sky
{

TransformTestScene::TransformTestScene( const GameMain* game_main )
	: Scene( game_main )
	, camera_( EntityManager::get_instance()->create_entity() )
{
	get_graphics_manager()->setup_default_shaders();
	get_graphics_manager()->load_paper_textures();

	get_graphics_manager()->load_model( "wall-1" );

	// System を追加する
	get_entity_manager()->add_system< core::ecs::TransformControlSystem >();
	// get_entity_manager()->add_system< core::ecs::RenderSystem >( 1000 - 1 );
	get_entity_manager()->add_system< core::ecs::RenderSystem >( 1000 );

	// Entity と Component を追加する
	cube_ = get_entity_manager()->create_entity();
	cube_transform_ = cube_->add_component< core::ecs::TransformComponent >();
	cube_transform_->transform.set_identity();
	cube_->add_component< core::ecs::RenderComponent >();

	/*
	cube_ = get_entity_manager()->create_entity();
	cube_transform_ = cube_->add_component< core::ecs::TransformComponent >();
	cube_transform_->transform.set_position( Vector( 3.f, 0.f, 0.f ) );
	cube_->add_component< core::ecs::RenderComponent >();
	cube_->add_component< core::ecs::TransformControlComponent >();
	*/

	camera_transform_ = camera_->add_component< core::ecs::TransformComponent >();
	camera_transform_->transform.set_identity();
	camera_transform_->transform.set_position( Vector( 0.f, 1.5f, -10.f ) );

	camera_->add_component< core::ecs::TransformControlComponent >();
	const auto* camera_component = camera_->add_component< core::ecs::CameraComponent >();

	get_graphics_manager()->set_main_camera_info( camera_transform_, camera_component );

	/*
	camera_->remove_component< core::ecs::CameraComponent >();
	cube_->add_component< core::ecs::CameraComponent >();

	get_graphics_manager()->set_main_camera( cube_ );
	*/
}

TransformTestScene::~TransformTestScene()
{
	get_entity_manager()->clear();
}

TransformTestScene::EntityManager* TransformTestScene::get_entity_manager()
{
	return EntityManager::get_instance();
}

void TransformTestScene::update()
{
	Scene::update();

	get_graphics_manager()->update();
	get_graphics_manager()->clear_debug_bullet();

	get_entity_manager()->update();

	// GUI によってオブジェクトの位置・回転を変更する
	/// @todo Component の値を GUI で変更する仕組みを整理する
	static Vector rot;

	ImGui::Begin( "transform test params" );
	ImGui::DragFloat3( "Position", reinterpret_cast< float* >( & cube_transform_->transform.get_position() ), 0.1f );
	ImGui::DragFloat3( "Rotation", reinterpret_cast< float* >( & rot ), 0.1f, -360.f, 360.f );
	ImGui::End();

	Quaternion q;
	Vector r = math::degree_to_radian( rot );
	q.set_euler_zyx( r.z(), r.y(), r.x() );
	cube_transform_->transform.set_rotation( q );
}

void TransformTestScene::render()
{
	std::stringstream ss;
	ss << "Time : " << get_total_elapsed_time() << '\n';
	ss << "FPS : " << get_main_loop()->get_last_fps() << '\n';
	ss << "pass count : " << get_graphics_manager()->get_pass_count() << '\n';
	ss << "draw count : " << get_graphics_manager()->get_draw_count() << '\n';

	ss << "eye : "
		<< camera_transform_->transform.get_position().x() << ", "
		<< camera_transform_->transform.get_position().y() << ", "
		<< camera_transform_->transform.get_position().z() << '\n';
	// ss << "rot : " << camera_->rotate_degree().x() << ", " << camera_->rotate_degree().y() << ", " << camera_->rotate_degree().z() << '\n';

	ss << "psts "<< '\n';

	get_graphics_manager()->draw_text( 10.f, 10.f, get_width() - 10.f, get_height() - 10.f, ss.str().c_str(), Color::White );
}

} // namespace blue_sky
