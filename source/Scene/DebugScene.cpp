#include "DebugScene.h"
#include "StageSelectScene.h"

#include <GraphicsManager.h>
#include <ActiveObjectManager.h>
#include <ActiveObjectPhysics.h>
#include <ScriptManager.h>
#include <Input.h>

#include <core/graphics/Direct3D11/Direct3D11Fader.h>

#include <common/math.h>

namespace blue_sky
{

DebugScene::DebugScene( const GameMain* game_main )
	: Scene( game_main )
	, camera_( new Camera() )
{
	// Physics
	get_physics()->add_ground_rigid_body( ActiveObject::Vector3( 1000, 1, 1000 ) );

	camera_->position().set( 0.f, 0.f, -10.f );

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
		camera_->position().x() -= moving_speed;
	}
	if ( get_input()->press( Input::RIGHT ) )
	{
		camera_->position().x() += moving_speed;
	}
	if ( get_input()->press( Input::UP ) )
	{
		camera_->position().z() += moving_speed;
	}
	if ( get_input()->press( Input::DOWN ) )
	{
		camera_->position().z() -= moving_speed;
	}
	if ( get_input()->press( Input::L ) )
	{
		camera_->position().y() -= moving_speed;
	}
	if ( get_input()->press( Input::R ) )
	{
		camera_->position().y() += moving_speed;
	}

	camera_->update();

	get_active_object_manager()->update();

	get_graphics_manager()->update();
	get_graphics_manager()->set_eye_position( camera_->position() );

	get_physics()->update( get_elapsed_time() );
}

void DebugScene::render()
{
	FrameConstantBufferData frame_constant_buffer_data;

	Vector eye( camera_->position().x(), camera_->position().y(), camera_->position().z() );
	Vector at( camera_->look_at().x(), camera_->look_at().y(), camera_->look_at().z() );
//	Vector at( 0.f, 0.f, 0.f );
	Vector up( camera_->up().x(), camera_->up().y(), camera_->up().z() );

	frame_constant_buffer_data.view = ( Matrix().set_look_at( eye, at, up ) ).transpose();
	frame_constant_buffer_data.projection = Matrix().set_perspective_fov( math::degree_to_radian( camera_->fov() ), camera_->aspect(), camera_->near_clip(), camera_->far_clip() ).transpose();

	get_graphics_manager()->get_frame_render_data()->update( & frame_constant_buffer_data );

	get_graphics_manager()->setup_rendering();
	get_graphics_manager()->render_background();
	get_graphics_manager()->render_active_objects( get_active_object_manager() );

	get_graphics_manager()->render_fader();

	get_graphics_manager()->render_debug_axis( get_active_object_manager() );
	get_graphics_manager()->render_debug_bullet();

	std::stringstream ss;
	ss << "eye : " << eye.x() << ", " << eye.y() << ", " << eye.z() << std::endl;
	ss << "fade :" << get_direct_3d()->getFader()->get_color().a() << std::endl; /// @todo フェーディングがスムーズでない原因を調べる

	get_graphics_manager()->draw_text( 10.f, 10.f, get_width() - 10.f, get_height() - 10.f, ss.str().c_str(), Color::White );
}

} // namespace blue_sky
