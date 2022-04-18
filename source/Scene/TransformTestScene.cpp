#include "TransformTestScene.h"

#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/ActiveObjectManager.h>
#include <blue_sky/ActiveObjectPhysics.h>
#include <blue_sky/Input.h>

#include <core/ecs/Component/TransformComponent.h>
#include <core/ecs/Component/RenderComponent.h>
#include <core/ecs/System/RenderSystem.h>
#include <core/ecs/EntityManager.h>

#include <game/MainLoop.h>

#include <common/math.h>

#include <imgui.h>

#include <iostream>

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
	get_entity_manager()->add_system< core::ecs::RenderSystem >( 1000 );

	// Entity と Component を追加する
	auto cube = get_entity_manager()->create_entity();
	cube->add_component< core::ecs::TransformComponent >()->transform.set_identity();
	cube->add_component< core::ecs::RenderComponent >();

	auto camera_transform = camera_->add_component< core::ecs::TransformComponent >();
	camera_transform->transform.set_identity();
	camera_transform->transform.set_position( Vector( 0.f, 1.5f, -10.f ) );
}

TransformTestScene::~TransformTestScene()
{
	get_active_object_manager()->clear();
}

TransformTestScene::EntityManager* TransformTestScene::get_entity_manager()
{
	return EntityManager::get_instance();
}

void TransformTestScene::update()
{
	Scene::update();

	get_entity_manager()->update();

	auto t = camera_->get_component< core::ecs::TransformComponent >();

	if ( get_input()->press( Input::Button::LEFT ) )
	{
		t->transform.get_position() -= t->transform.right() * 0.001f;
	}
	if ( get_input()->press( Input::Button::RIGHT) )
	{
		t->transform.get_position() += t->transform.right() * 0.001f;
	}

	t->transform.set_rotation( Quaternion( get_input()->get_mouse_dx(), get_input()->get_mouse_dy(), 0.f ) *  t->transform.get_rotation() );

#ifdef ECS
	camera_->rotate_degree_target() += Vector( get_input()->get_mouse_dy() * 90.f, get_input()->get_mouse_dx() * 90.f, 0.f );
	camera_->rotate_degree_target().set_x( math::clamp( camera_->rotate_degree_target().x(), -90.f, +90.f ) );

	const float moving_speed = get_input()->press( Input::Button::R2 ) ? 0.3f : 0.1f;

	
	if ( get_input()->press( Input::Button::UP ) )
	{
		camera_->position() += camera_->front() * moving_speed;
	}
	if ( get_input()->press( Input::Button::DOWN ) )
	{
		camera_->position() -= camera_->front() * moving_speed;
	}
	if ( get_input()->press( Input::Button::L ) )
	{
		camera_->position() += camera_->up() * moving_speed;
	}
	if ( get_input()->press( Input::Button::L2 ) )
	{
		camera_->position() -= camera_->up() * moving_speed;
	}

	camera_->position().set_y( std::max( camera_->position().y(), 0.1f ) );
	camera_->update();
#endif

	get_active_object_manager()->update();

	get_graphics_manager()->update();

	get_graphics_manager()->set_eye_position( t->transform.get_position() );

	get_graphics_manager()->clear_debug_bullet();

	static float pos[ 3 ] = { 0 };
	static float rot[ 3 ] = { 0 };

	ImGui::Begin( "transform test params" );
	ImGui::InputFloat3( "Position", pos );
	ImGui::InputFloat3( "Rotation", rot );
	ImGui::End();
}

void TransformTestScene::render()
{
	auto& frame_render_data = get_graphics_manager()->get_frame_render_data()->data();

	auto ct = camera_->get_component< core::ecs::TransformComponent >()->transform;
	// auto c = camera_->get_component< core::ecs::CameraComponent >();

	const auto& eye = ct.get_position();
	const auto at = ct.get_position() + ct.forward();
	const auto up = ct.up();

	frame_render_data.view = ( Matrix().set_look_at( eye, at, up ) ).transpose();
	frame_render_data.projection = Matrix().set_perspective_fov( math::degree_to_radian( 90.f ), static_cast< float >( get_width() ) / static_cast< float >( get_height() ), 0.05f, 1500.f ).transpose();
	frame_render_data.light = Vector( -1.f, -2.f, 0.f, 0.f ).normalize();

	get_graphics_manager()->get_frame_render_data()->update();

	get_graphics_manager()->setup_rendering();

	get_graphics_manager()->render_background();
	get_graphics_manager()->render_active_objects( get_active_object_manager() );

	/// @todo 整理する。
	get_entity_manager()->update();

	get_graphics_manager()->render_fader();

	get_graphics_manager()->render_debug_axis( get_active_object_manager() );
	get_graphics_manager()->render_debug_bullet();

	std::stringstream ss;
	ss << "Time : " << get_total_elapsed_time() << '\n';
	ss << "FPS : " << get_main_loop()->get_last_fps() << '\n';
	ss << "pass count : " << get_graphics_manager()->get_pass_count() << '\n';
	ss << "draw count : " << get_graphics_manager()->get_draw_count() << '\n';

#ifdef ECS
	ss << "eye : " << eye.x() << ", " << eye.y() << ", " << eye.z() << '\n';
	ss << "rot : " << camera_->rotate_degree().x() << ", " << camera_->rotate_degree().y() << ", " << camera_->rotate_degree().z() << '\n';
#endif

	ss << "psts "<< '\n';

	get_graphics_manager()->draw_text( 10.f, 10.f, get_width() - 10.f, get_height() - 10.f, ss.str().c_str(), Color::White );
}

} // namespace blue_sky
