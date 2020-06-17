#include "DebugScene.h"
#include "StageSelectScene.h"

#include <GameObject/Camera.h>

#include <ActiveObjectManager.h>
#include <ActiveObjectPhysics.h>
#include <ScriptManager.h>
#include <Input.h>

#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/graphics/shader/post_effect/HandDrawingShader.h>

#include <core/graphics/Sprite.h>
#include <core/graphics/RenderTargetTexture.h>

#include <game/MainLoop.h>

#include <common/math.h>

#include <imgui.h>

namespace blue_sky
{

DebugScene::DebugScene( const GameMain* game_main )
	: Scene( game_main )
	, camera_( new Camera() )
	, render_result_texture_( get_graphics_manager()->create_render_target_texture() )
{
	// Physics
	get_physics_manager()->add_ground_rigid_body( Vector( 1000, 1, 1000 ) );

	get_graphics_manager()->setup_default_shaders();

	camera_->position().set( 0.f, 1.5f, -10.f, 1.f );

	auto* city = get_active_object_manager()->create_object( "static" );
	city->set_model( city_generator_.get_model() );

	get_script_manager()->exec( "load( \"test/init.lua\" )" );

	get_graphics_manager()->load_named_texture( "2x2", "media/texture/rgby.png" );
}

DebugScene::~DebugScene()
{
	get_active_object_manager()->clear();
}

void DebugScene::update()
{
	Scene::update();

	camera_->rotate_degree_target() += Vector( get_input()->get_mouse_dy() * 90.f, get_input()->get_mouse_dx() * 90.f, 0.f );
	camera_->rotate_degree_target().set_x( math::clamp( camera_->rotate_degree_target().x(), -90.f, +90.f ) );

	const float moving_speed = 0.1f;

	if ( get_input()->press( Input::LEFT ) )
	{
		camera_->position() -= camera_->right() * moving_speed;
	}
	if ( get_input()->press( Input::RIGHT ) )
	{
		camera_->position() += camera_->right() * moving_speed;
	}
	if ( get_input()->press( Input::UP ) )
	{
		camera_->position() += camera_->front() * moving_speed;
	}
	if ( get_input()->press( Input::DOWN ) )
	{
		camera_->position() -= camera_->front() * moving_speed;
	}
	if ( get_input()->press( Input::L ) )
	{
		camera_->position() += camera_->up() * moving_speed;
	}
	if ( get_input()->press( Input::L2 ) )
	{
		camera_->position() -= camera_->up() * moving_speed;
	}

	camera_->position().set_y( std::max( camera_->position().y(), 0.1f ) );

	if ( get_input()->push( Input::A ) )
	{
		city_generator_.step();

		// GameObject* o = get_active_object_manager()->get_nearest_object( GameObject::Vector3( camera_->position().x(), camera_->position().y(), camera_->position().z() ) );
		// o->get_component< 

	}

	// tess test
	{
		if ( get_input()->press( Input::R2 ) )
		{
			get_graphics_manager()->get_frame_render_data()->data().tess_factor -= 1 * get_elapsed_time();
		}
		if ( get_input()->press( Input::R ) )
		{
			get_graphics_manager()->get_frame_render_data()->data().tess_factor += 1 * get_elapsed_time();
		}

		get_graphics_manager()->get_frame_render_data()->data().tess_factor = math::clamp( get_graphics_manager()->get_frame_render_data()->data().tess_factor, 1.f, 8.f );
		get_graphics_manager()->get_frame_render_data()->data().time = get_total_elapsed_time();
	}

	camera_->update();

	get_active_object_manager()->update();

	get_graphics_manager()->update();
	get_graphics_manager()->set_eye_position( camera_->position() );

	get_graphics_manager()->clear_debug_bullet();
	get_physics_manager()->update( get_elapsed_time() );

	ImGui::Begin( "HandDrawingShader params" );

	/*
	ImGui::Text("Hello, world %d", 123);
	
	if ( ImGui::Button( "Save" ) )
	{
		
	}

	static float my_color[ 4 ] = { 0.f };
	ImGui::ColorEdit4("Color", my_color);

	static std::string s( 256, 0 );
	ImGui::InputText("string", s.data(), s.size());
	*/

	auto* hand_drawing_shader = get_graphics_manager()->get_shader< graphics::shader::post_effect::HandDrawingShader >( "post_effect_hand_drawing" );
	ImGui::SliderFloat( "UvFactor", & hand_drawing_shader->getUvFactor(), 0.f, 100.f );
	ImGui::SliderFloat( "TimeFactor", & hand_drawing_shader->getTimeFactor(), 0.f, 100.f );
	ImGui::SliderFloat( "Gain1", & hand_drawing_shader->getGain1(), 0.00001f, 0.01f, "%.5f" );
	ImGui::SliderFloat( "Gain2", & hand_drawing_shader->getGain2(), 0.00001f, 0.01f, "%.5f" );
	ImGui::SliderFloat( "Gain3", & hand_drawing_shader->getGain3(), 0.00001f, 0.01f, "%.5f" );

	ImGui::End();
}

void DebugScene::render()
{
	auto& frame_render_data = get_graphics_manager()->get_frame_render_data()->data();

	Vector eye( camera_->position().x(), camera_->position().y(), camera_->position().z(), 1.f );
	Vector at( camera_->look_at().x(), camera_->look_at().y(), camera_->look_at().z(), 1.f );
	Vector up( camera_->up().x(), camera_->up().y(), camera_->up().z(), 0.f );

	frame_render_data.view = ( Matrix().set_look_at( eye, at, up ) ).transpose();
	frame_render_data.projection = Matrix().set_perspective_fov( math::degree_to_radian( camera_->fov() ), camera_->aspect(), camera_->near_clip(), camera_->far_clip() ).transpose();
	frame_render_data.light = Vector( -1.f, -2.f, 0.f, 0.f ).normalize();

	get_graphics_manager()->get_frame_render_data()->update();

	get_graphics_manager()->setup_rendering();

	get_graphics_manager()->set_render_target( render_result_texture_.get() );

	get_graphics_manager()->render_background();
	get_graphics_manager()->render_active_objects( get_active_object_manager() );

	get_graphics_manager()->render_post_effect( render_result_texture_.get() );

	get_graphics_manager()->render_fader();

	get_graphics_manager()->render_debug_axis( get_active_object_manager() );
	get_graphics_manager()->render_debug_bullet();

	if ( get_input()->press( Input::B ) )
	{
		get_graphics_manager()->unset_depth_stencil();
		get_graphics_manager()->resolve_depth_texture();

		/// @todo sprite_ms ‚Å‚Ì•`‰æ‚É‘Î‰ž‚·‚é
		get_graphics_manager()->get_sprite()->begin();
		get_graphics_manager()->get_sprite()->draw( win::Rect( get_width() / 4.f, get_height() / 4.f, get_width() / 4.f * 3.f, get_height() / 4.f * 3.f ), get_graphics_manager()->get_depth_texture() );
		// get_graphics_manager()->get_sprite()->draw( win::Rect( get_width() / 4.f, get_height() / 4.f, get_width() / 4.f * 3.f, get_height() / 4.f * 3.f ), get_graphics_manager()->get_texture( "2x2" ) );
		get_graphics_manager()->get_sprite()->end();

		get_graphics_manager()->set_depth_stencil();
	}

	std::stringstream ss;
	ss << "Time : " << get_total_elapsed_time() << '\n';
	ss << "FPS : " << get_main_loop()->get_last_fps() << '\n';
	ss << "pass count : " << get_graphics_manager()->get_pass_count() << '\n';
	ss << "draw count : " << get_graphics_manager()->get_draw_count() << '\n';

	ss << "eye : " << eye.x() << ", " << eye.y() << ", " << eye.z() << '\n';
	ss << "rot : " << camera_->rotate_degree().x() << ", " << camera_->rotate_degree().y() << ", " << camera_->rotate_degree().z() << '\n';

	ss << "tess : " << frame_render_data.tess_factor << '\n';

	get_graphics_manager()->draw_text( 10.f, 10.f, get_width() - 10.f, get_height() - 10.f, ss.str().c_str(), Color::White );
}

} // namespace blue_sky
