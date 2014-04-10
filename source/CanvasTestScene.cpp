#include "CanvasTestScene.h"
#include "Direct3D11.h"
#include "Direct3D11Mesh.h"
#include "GraphicsManager.h"
#include "DrawingModel.h"
#include "FbxFileLoader.h"
#include "GameMain.h"
#include "memory.h"

namespace blue_sky
{

CanvasTestScene::CanvasTestScene( const GameMain* game_main )
	: Scene( game_main )
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
		XMVECTOR eye = XMVectorSet( -3.f, 3.f, -3.f, 1.f );
		XMVECTOR at = XMVectorSet( 0.f, 0.f, 0.f, 1.f );
		XMVECTOR up = XMVectorSet( 0.f, 1.f, 0.f, 1.f );

		FrameConstantBufferData frame_constant_buffer_data;

		frame_constant_buffer_data.view = XMMatrixLookAtLH( eye, at, up );
		frame_constant_buffer_data.view = XMMatrixTranspose( frame_constant_buffer_data.view );
		frame_constant_buffer_data.projection = XMMatrixPerspectiveFovLH( math::degree_to_radian( 90.f ), static_cast< float >( get_width() ) / static_cast< float >( get_height() ), 0.05f, 3000.f );
		frame_constant_buffer_data.projection = XMMatrixTranspose( frame_constant_buffer_data.projection );
		frame_constant_buffer_data.light = Vector( 0, 0, 0 );
		frame_constant_buffer_data.time = 0.f;
		frame_constant_buffer_data.time_beat = 0;

		get_game_main()->get_frame_constant_buffer()->update( & frame_constant_buffer_data );
	}
	{
		Matrix m;

		m.set_identity();
		m.set_translation( 0.f, 0.f, 0.f );

		ObjectConstantBufferData buffer_data;
		buffer_data.world = m.inverse();
		buffer_data.color = Color( 1.f, 0.f, 0.f, 1.f );

		get_game_main()->get_object_constant_buffer()->update( & buffer_data );
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
}

/**
 * •`‰æ
 *
 */
void CanvasTestScene::render()
{
	get_direct_3d()->begin3D();

	get_direct_3d()->clear();

	get_direct_3d()->set_default_render_target();
	get_direct_3d()->set_default_viewport();

	get_direct_3d()->setInputLayout( "main" );

	render_technique( "|main", [&] () {
		get_game_main()->get_game_constant_buffer()->bind_to_all();
	
		get_game_main()->get_frame_constant_buffer()->bind_to_all();

		get_game_main()->get_object_constant_buffer()->bind_to_vs();
		get_game_main()->get_object_constant_buffer()->bind_to_ps();
		
		mesh_->render();
	} );

	get_direct_3d()->end3D();
}

} // namespace blue_sky
