#include "CanvasTestScene.h"
#include "Direct3D11.h"
#include "Direct3D11Mesh.h"
#include "Direct3D11Sprite.h"
#include "Direct3D11Texture.h"
#include "Direct3D11SkyBox.h"
#include "GraphicsManager.h"
#include "DrawingModel.h"
#include "FbxFileLoader.h"
#include "GameMain.h"

#include "DirectWrite.h"

#include "Input.h"

#include <game/Texture.h>
#include <game/MainLoop.h>

#include <win/Rect.h>

#include <common/math.h>
#include <common/random.h>

#include "memory.h"

namespace blue_sky
{

static FrameConstantBufferData frame_constant_buffer_data;
static ObjectConstantBufferData object_constant_buffer_data;
static Vector eye( 0.f, 0.f, -1.f );
static Matrix r;

static int pen_mode_ = 0;

CanvasTestScene::CanvasTestScene( const GameMain* game_main )
	: Scene( game_main )
	, tablet_( Tablet::get_instance( get_game_main()->get_app()->GetWindowHandle() ) )
	, pen_color_( 0.f, 0.f, 0.f, 1.f )
	, points_( new DynamicPointList( get_direct_3d() ) )
	, sky_box_( new SkyBox( get_direct_3d(), "sky-box-sky" ) )
{
	{
		GameConstantBufferData constant_buffer_data;
		constant_buffer_data.screen_width = static_cast< float_t >( get_width() );
		constant_buffer_data.screen_height = static_cast< float_t >( get_height() );
		
		get_game_main()->get_game_constant_buffer()->update( & constant_buffer_data );
	}

	texture_ = get_graphics_manager()->get_texture( "white-hard-pen" );
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

	const float rs = 0.05f;
	static float ry = 0.f;
	static float rx = 0.f;

	static float ss = 0.01f;
	static float scale = 1.f;


	if ( get_input()->press( Input::LEFT ) )
	{
		ry -= rs;
	}

	if ( get_input()->press( Input::RIGHT ) )
	{
		ry += rs;
	}

	if ( get_input()->press( Input::UP ) )
	{
		rx -= rs;
	}

	if ( get_input()->press( Input::DOWN ) )
	{
		rx += rs;
	}

	if ( get_input()->press( Input::L ) )
	{
		scale *= 0.98f;
	}

	if ( get_input()->press( Input::R ) )
	{
		scale *= 1.02f;
	}

	scale = math::clamp( scale, 0.1f, 50.f );

	if ( pen_mode_ == 1 && get_input()->release( Input::A ) || pen_mode_ == 2 )
	{
		pen_color_ = Color( common::random( 0.f, 1.f ), common::random( 0.f, 1.f ), common::random( 0.f, 1.f ), 1.f );
	}

	{
		static float v = 0.f;

		if ( get_input()->press( Input::B ) )
		{
			v += 0.005f;

			if ( v > 1.f )
			{
				v -= 1.f;
			}

			pen_color_ = Color::from_hsv( v, 1.f, 1.f );
		}
	}

	if ( get_input()->press( Input::A ) )
	{
		const float dc = 0.02f;

		pen_color_.r() += common::random( -dc, +dc );
		pen_color_.g() += common::random( -dc, +dc );
		pen_color_.b() += common::random( -dc, +dc );
	}

	{
		eye = Vector( 0.f, 0.f, -1.f * scale );
		Vector at( 0.f, 0.f, 0.f );
		Vector up( 0.f, 1.f, 0.f );

		frame_constant_buffer_data.view.set_look_at( eye, at, up );
		frame_constant_buffer_data.view = frame_constant_buffer_data.view.transpose();

		frame_constant_buffer_data.projection.set_perspective_fov( math::degree_to_radian( 90.f ), static_cast< float >( get_width() ) / static_cast< float >( get_height() ), 0.05f, 3000.f );
		frame_constant_buffer_data.projection = frame_constant_buffer_data.projection.transpose();

		frame_constant_buffer_data.light = Vector( 0, 0, 0 );

		if ( static_cast< int >( get_total_elapsed_time() * 60.f ) % 10 == 0 )
		{
			frame_constant_buffer_data.time = common::random( -5.f, 5.f );
		}
		
		frame_constant_buffer_data.time_beat = 0;

		get_game_main()->get_frame_constant_buffer()->update( &frame_constant_buffer_data );
	}

	Matrix s;

	{
		r.set_rotation_xyz( rx, ry, 0.f );
		s.set_scaling( scale, scale, scale );

		object_constant_buffer_data.world = /* s * */ r;
		object_constant_buffer_data.color = Color( 1.f, 1.f, 1.f, 1.f );
	}

	Vector vector( tablet_->get_x() / get_width() - 0.5f, -( tablet_->get_y() / get_height() - 0.5f ), 0.f );

	Matrix m = r;


	// Matrix m = s * r * frame_constant_buffer_data.view * frame_constant_buffer_data.projection;
	// m.inverse();

	// Matrix m = frame_constant_buffer_data.projection.inverse() * frame_constant_buffer_data.view.inverse() * r.inverse() * s.inverse();

	vector *= m;

	Vertex v
	{
		Vector3( vector.x(), vector.y(), vector.z() ),
		get_input()->press( Input::A ) ? std::pow( tablet_->get_pressure(), 2 ) : 0.5f,
		pen_color_
	};

	auto pos_comp = [] ( const Vertex& a, const Vertex& b ) { return a.position.x == b.position.x && a.position.y == b.position.y && a.position.z == b.position.z; };

	points_->update_last_point( v, pos_comp );

	if ( get_input()->press( Input::A ) )
	{
		if ( tablet_->get_cursor_index() == 1 )
		{
			points_->add_point( v, pos_comp );
		}
		else if ( tablet_->get_cursor_index() == 2 )
		{
			points_->erase_point( [this,&vector] ( const Vertex& a ) { Vector aa( a.position.x, a.position.y, a.position.z ); return ( vector - aa ).length() < tablet_->get_pressure() * 0.1f; } );
		}
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

		ss << L"Time : " << get_total_elapsed_time() << std::endl;
		ss << L"FPS : " << get_main_loop()->get_last_fps() << std::endl;
		ss << L"C : " << tablet_->get_cursor_index() << std::endl;
		ss << L"X : " << tablet_->get_x() << std::endl;
		ss << L"Y : " << tablet_->get_y() << std::endl;
		ss << L"P : " << tablet_->get_pressure() << std::endl;
		ss << L"AZ : " << tablet_->get_azimuth() << std::endl;
		ss << L"AL : " << tablet_->get_altitude() << std::endl;
		ss << L"POINTS : " << points_->size() << std::endl;

		get_direct_3d()->getFont()->draw_text( 10.f, 10.f, get_app()->get_width() - 10.f, get_app()->get_height() - 10.f, ss.str().c_str(), Direct3D::Color( 0.f, 0.f, 0.f, 1.f ) );

		get_direct_3d()->getFont()->end();
		get_direct_3d()->end2D();
	}

	get_direct_3d()->begin3D();

	get_direct_3d()->clear();

	get_direct_3d()->set_default_render_target();
	get_direct_3d()->set_default_viewport();

	get_direct_3d()->setInputLayout( "main" );

	render_technique( "|sky_box", [&] {
		
		get_game_main()->get_game_constant_buffer()->bind_to_all();
		get_game_main()->get_frame_constant_buffer()->bind_to_all();
		get_game_main()->get_object_constant_buffer()->bind_to_all();

		ObjectConstantBufferData object_constant_buffer_data;
		object_constant_buffer_data.world = Matrix().set_translation( eye.x(), eye.y(), eye.z() ).transpose() * r;
		object_constant_buffer_data.color = Color( 1.f, 1.f, 1.f, 1.f );

		get_game_main()->get_object_constant_buffer()->update( &object_constant_buffer_data );

		sky_box_->render();
	} );


	get_direct_3d()->setInputLayout( "drawing_point" );

	render_technique( "|drawing_point", [&] {
		get_game_main()->get_game_constant_buffer()->bind_to_all();
	
		get_game_main()->get_frame_constant_buffer()->bind_to_all();

		get_game_main()->get_object_constant_buffer()->bind_to_vs();
		get_game_main()->get_object_constant_buffer()->bind_to_ps();
		
		get_game_main()->get_object_constant_buffer()->update( &object_constant_buffer_data );

		texture_->bind_to_ps( 1 );
		points_->render();
	} );

	get_direct_3d()->renderText();

	get_direct_3d()->end3D();
}

void CanvasTestScene::on_function_key_down( int key )
{
	if ( key == 8 )
	{
		points_->clear();
	}

	if ( key == 9 )
	{
		const std::array< char_t*, 5 > texture_names = { "white-soft-pen", "white-hard-pen", "bump-hard-pen", "bump-cross-pen", "white-grass-pen" };

		static int i = 0;
		i = ( i + 1 ) % texture_names.size();;

		texture_ = get_graphics_manager()->get_texture( texture_names[ i ] );
	}

	if ( key == 6 )
	{
		pen_mode_ = ( pen_mode_ + 1 ) % 3;
	}
}

} // namespace blue_sky
