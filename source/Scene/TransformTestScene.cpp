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
	camera_->add_component< core::ecs::CameraComponent >();
}

TransformTestScene::~TransformTestScene()
{
	
}

TransformTestScene::EntityManager* TransformTestScene::get_entity_manager()
{
	return EntityManager::get_instance();
}

void TransformTestScene::update()
{
	Scene::update();

	get_entity_manager()->update();

#ifdef ECS
	camera_->rotate_degree_target() += Vector( get_input()->get_mouse_dy() * 90.f, get_input()->get_mouse_dx() * 90.f, 0.f );
	camera_->rotate_degree_target().set_x( math::clamp( camera_->rotate_degree_target().x(), -90.f, +90.f ) );
#endif

	get_graphics_manager()->update();

	get_graphics_manager()->set_eye_position( camera_transform_->transform.get_position() );

	get_graphics_manager()->clear_debug_bullet();

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
	auto& frame_render_data = get_graphics_manager()->get_frame_render_data()->data();

	const auto& camera_transform = camera_->get_component< core::ecs::TransformComponent >()->transform;
	const auto& camera_component = camera_->get_component< core::ecs::CameraComponent >();

	const auto& eye = camera_transform.get_position();
	const auto at = camera_transform.get_position() + camera_transform.forward();
	const auto up = camera_transform.up();

	/// @todo FOV, ニアクリップ平面, ファークリップ平面の情報を CameraComponent から取得する
	frame_render_data.view = ( Matrix().set_look_at( eye, at, up ) ).transpose();
	frame_render_data.projection = Matrix().set_perspective_fov( math::degree_to_radian( camera_component->fov ), static_cast< float >( get_width() ) / static_cast< float >( get_height() ), camera_component->near_clip, camera_component->far_clip ).transpose();
	frame_render_data.light = Vector( -1.f, -2.f, 0.f, 0.f ).normalize();

	get_graphics_manager()->get_frame_render_data()->update();

	get_graphics_manager()->setup_rendering();

	get_graphics_manager()->render_background();

	/// @todo 整理する。
	get_entity_manager()->update();

	get_graphics_manager()->render_fader();
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
