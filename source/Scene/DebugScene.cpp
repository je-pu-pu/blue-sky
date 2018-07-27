#include "DebugScene.h"
#include "StageSelectScene.h"

#include <ActiveObjectManager.h>
#include <ActiveObjectPhysics.h>
#include <ScriptManager.h>
#include <Input.h>

#include <blue_sky/graphics/GraphicsManager.h>

#include <common/math.h>

namespace blue_sky
{

DebugScene::DebugScene( const GameMain* game_main )
	: Scene( game_main )
	, camera_( new Camera() )
{
	// Physics
	get_physics_manager()->add_ground_rigid_body( ActiveObject::Vector3( 1000, 1, 1000 ) );

	get_graphics_manager()->setup_default_shaders();

	camera_->position().set( 0.f, 0.f, -10.f, 1.f );

	// new RenderData< >();
	// new Direct3D11ConstantBuffer< 
}

DebugScene::~DebugScene()
{

}

void DebugScene::update()
{
	Scene::update();

	const float moving_speed = 0.1f;

	if ( get_input()->press( Input::LEFT ) )
	{
		camera_->position() -= Vector( moving_speed, 0.f, 0.f, 0.f );
	}
	if ( get_input()->press( Input::RIGHT ) )
	{
		camera_->position() += Vector( moving_speed, 0.f, 0.f, 0.f );
	}
	if ( get_input()->press( Input::UP ) )
	{
		camera_->position() += Vector( 0.f, 0.f, moving_speed, 0.f );
	}
	if ( get_input()->press( Input::DOWN ) )
	{
		camera_->position() -= Vector( 0.f, 0.f, moving_speed, 0.f );
	}
	if ( get_input()->press( Input::L ) )
	{
		camera_->position() -= Vector( 0.f, moving_speed, 0.f, 0.f );
	}
	if ( get_input()->press( Input::R ) )
	{
		camera_->position() += Vector( 0.f, moving_speed, 0.f, 0.f );
	}

	if ( get_input()->push( Input::A ) )
	{
		// GameObject* o = get_active_object_manager()->get_nearest_object( GameObject::Vector3( camera_->position().x(), camera_->position().y(), camera_->position().z() ) );
		// o->get_component< 

	}

	// tess test
	{
		if ( get_input()->press( Input::L2 ) )
		{
			get_graphics_manager()->get_frame_render_data()->data().tess_factor -= 1 * get_elapsed_time();
		}
		if ( get_input()->press( Input::R2 ) )
		{
			get_graphics_manager()->get_frame_render_data()->data().tess_factor += 1 * get_elapsed_time();
		}

		get_graphics_manager()->get_frame_render_data()->data().tess_factor = math::clamp( get_graphics_manager()->get_frame_render_data()->data().tess_factor, 1.f, 8.f );
	}

	camera_->update();

	get_active_object_manager()->update();

	get_graphics_manager()->update();
	get_graphics_manager()->set_eye_position( camera_->position().xyz() );

	get_graphics_manager()->clear_debug_bullet();
	get_physics_manager()->update( get_elapsed_time() );
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
	get_graphics_manager()->render_background();
	get_graphics_manager()->render_active_objects( get_active_object_manager() );

	get_graphics_manager()->render_fader();

	get_graphics_manager()->render_debug_axis( get_active_object_manager() );
	get_graphics_manager()->render_debug_bullet();

	std::stringstream ss;
	ss << "eye : " << eye.x() << ", " << eye.y() << ", " << eye.z() << std::endl;
	ss << "tess : " << frame_render_data.tess_factor << std::endl;

	get_graphics_manager()->draw_text( 10.f, 10.f, get_width() - 10.f, get_height() - 10.f, ss.str().c_str(), Color::White );
}

} // namespace blue_sky
