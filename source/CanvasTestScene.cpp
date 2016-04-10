#include "CanvasTestScene.h"
#include "Direct3D11.h"
#include "Direct3D11Mesh.h"
#include "Direct3D11Sprite.h"
#include "Direct3D11Texture.h"
#include "GraphicsManager.h"
#include "DrawingModel.h"
#include "FbxFileLoader.h"
#include "GameMain.h"

#include "DirectWrite.h"

#include <game/Texture.h>

#include <win/Rect.h>

#include "memory.h"

namespace blue_sky
{

CanvasTestScene::CanvasTestScene( const GameMain* game_main )
	: Scene( game_main )
	, tablet_( Tablet::get_instance( get_game_main()->get_app()->GetWindowHandle() ) )
{
	FbxFileLoader fbx_file_loader;

	mesh_.reset( get_graphics_manager()->create_mesh() );
	
	if ( ! mesh_->load_fbx( "media/model/box-2x2x2.fbx", & fbx_file_loader ) )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( "no" );
	}

	{
		GameConstantBufferData constant_buffer_data;
		constant_buffer_data.screen_width = static_cast< float_t >( get_width() );
		constant_buffer_data.screen_height = static_cast< float_t >( get_height() );
		
		get_game_main()->get_game_constant_buffer()->update( & constant_buffer_data );
	}
	{
		Vector eye( -3.f, 3.f, -3.f );
		Vector at( 0.f, 0.f, 0.f );
		Vector up( 0.f, 1.f, 0.f );

		FrameConstantBufferData frame_constant_buffer_data;

		frame_constant_buffer_data.view.set_look_at( eye, at, up );
		frame_constant_buffer_data.view = frame_constant_buffer_data.view.transpose();

		frame_constant_buffer_data.projection.set_perspective_fov( math::degree_to_radian( 90.f ), static_cast< float >( get_width() ) / static_cast< float >( get_height() ), 0.05f, 3000.f );
		frame_constant_buffer_data.projection = frame_constant_buffer_data.projection.transpose();
		
		frame_constant_buffer_data.light = Vector( 0, 0, 0 );
		frame_constant_buffer_data.time = 0.f;
		frame_constant_buffer_data.time_beat = 0;

		get_game_main()->get_frame_constant_buffer()->update( & frame_constant_buffer_data );
	}

	texture_ = get_graphics_manager()->get_texture( "pen" );
}

CanvasTestScene::~CanvasTestScene()
{
	
}

/**
 * ƒƒCƒ“ƒ‹[ƒvˆ—
 *
 */
void CanvasTestScene::update()
{
	Scene::update();

	if ( tablet_->get_pressure() > 0.f )
	{
		tablet_->get_x();
		tablet_->get_y();
	}

	{
		Matrix r, s;

		r.set_rotation_y( tablet_->get_pressure() );

		s.set_scaling( tablet_->get_x() * 0.01f, tablet_->get_y() * 0.01f, 1.f );

		ObjectConstantBufferData buffer_data;
		buffer_data.world = ( r * s ).inverse();
		buffer_data.color = Color( 1.f, 0.f, 0.f, 1.f );

		get_game_main()->get_object_constant_buffer()->update( & buffer_data );
	}
}

/**
 * •`‰æ
 *
 */
void CanvasTestScene::render()
{
	{
		get_direct_3d()->begin2D();
		get_direct_3d()->getFont()->begin();

		std::wstringstream ss;
		ss.setf( std::ios_base::fixed, std::ios_base::floatfield );

		ss << L"C : " << tablet_->get_cursor_index() << std::endl;
		ss << L"X : " << tablet_->get_x() << std::endl;
		ss << L"Y : " << tablet_->get_y() << std::endl;
		ss << L"P : " << tablet_->get_pressure() << std::endl;
		ss << L"AZ : " << tablet_->get_azimuth() << std::endl;
		ss << L"AL : " << tablet_->get_altitude() << std::endl;

		get_direct_3d()->getFont()->draw_text( 10.f, 10.f, get_app()->get_width() - 10.f, get_app()->get_height() - 10.f, ss.str().c_str(), Direct3D::Color( 0.f, 0.f, 0.f, 1.f ) );

		get_direct_3d()->getFont()->end();
		get_direct_3d()->end2D();
	}

	get_direct_3d()->begin3D();

	get_direct_3d()->clear();

	get_direct_3d()->set_default_render_target();
	get_direct_3d()->set_default_viewport();

	get_direct_3d()->setInputLayout( "main" );

	render_technique( "|main_canvas", [&] {
		get_game_main()->get_game_constant_buffer()->bind_to_all();
	
		get_game_main()->get_frame_constant_buffer()->bind_to_all();

		get_game_main()->get_object_constant_buffer()->bind_to_vs();
		get_game_main()->get_object_constant_buffer()->bind_to_ps();
		
		texture_->bind_to_ps( 1 );
		mesh_->render();
	} );

	{
		get_direct_3d()->getSprite()->begin();

		render_technique( "|sprite", [&] {
			get_direct_3d()->getSprite()->draw( win::Rect::Size( static_cast< int >( tablet_->get_x() ), static_cast< int >( tablet_->get_y() ), 16, 16 ), texture_.get(), win::Rect( 0, 0, 64, 64 ) );
		} );

		get_direct_3d()->getSprite()->end();
	}

	get_direct_3d()->renderText();

	get_direct_3d()->end3D();
}

} // namespace blue_sky
