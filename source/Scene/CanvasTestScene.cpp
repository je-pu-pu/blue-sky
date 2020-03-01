#include "CanvasTestScene.h"

#include "GameMain.h"

#include "Input.h"

#include <blue_sky/graphics/GraphicsManager.h>
#include <game/MainLoop.h>

#include <win/Rect.h>

#include <common/math.h>
#include <common/random.h>

namespace blue_sky
{

/// @todo ‚¿‚á‚ñ‚Æ‚·‚é
static ObjectConstantBufferData object_constant_buffer_data;
static Vector eye( 0.f, 0.f, -1.f, 1.f );
static Matrix r;

static int pen_mode_ = 0;

CanvasTestScene::CanvasTestScene( const GameMain* game_main )
	: Scene( game_main )
	, tablet_( Tablet::get_instance( get_game_main()->get_app()->GetWindowHandle() ) )
	, pen_color_( 0.f, 0.f, 0.f, 1.f )
	, points_( new DynamicPointList( get_direct_3d() ) )
{
	texture_ = get_graphics_manager()->load_texture( "media/texture/pen/white-hard-pen.png" );
	get_graphics_manager()->set_sky_box( "sky-box-sky" );
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
		if ( get_input()->press( Input::B ) )
		{
			static float v = 0.f;

			v += 0.005f;

			if ( v > 1.f )
			{
				v -= 1.f;
			}

			pen_color_ = Color::from_hsv( v, 1.f, 1.f );
		}

		if ( get_input()->press( Input::L2 ) )
		{
			pen_color_.r() -= 0.005f;
			pen_color_.g() -= 0.005f;
			pen_color_.b() -= 0.005f;
		}
		if ( get_input()->press( Input::R2 ) )
		{
			pen_color_.r() += 0.005f;
			pen_color_.g() += 0.005f;
			pen_color_.b() += 0.005f;
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
		eye = Vector( 0.f, 0.f, -1.f * scale, 1.f );
		Vector at( 0.f, 0.f, 0.f, 1.f );
		Vector up( 0.f, 1.f, 0.f, 0.f );

		auto& render_data = get_graphics_manager()->get_frame_render_data()->data();
		
		render_data.view.set_look_at( eye, at, up );
		render_data.view = render_data.view.transpose();

		render_data.projection.set_perspective_fov( math::degree_to_radian( 90.f ), static_cast< float >( get_width() ) / static_cast< float >( get_height() ), 0.05f, 3000.f );
		render_data.projection = render_data.projection.transpose();

		render_data.light = Vector( 0, 0, 0, 1 );

		if ( static_cast< int >( get_total_elapsed_time() * 60.f ) % 10 == 0 )
		{
			render_data.time = common::random( -5.f, 5.f );
		}
		
		render_data.time_beat = 0;

		get_graphics_manager()->set_eye_position( eye );
		get_graphics_manager()->get_frame_render_data()->update();
	}

	Matrix s;

	{
		r.set_rotation_xyz( rx, ry, 0.f );
		s.set_scaling( scale, scale, scale );

		object_constant_buffer_data.world = /* s * */ r;
		object_constant_buffer_data.color = Color( 1.f, 1.f, 1.f, 1.f );
	}

	Vector vector( tablet_->get_x() / get_width() - 0.5f, -( tablet_->get_y() / get_height() - 0.5f ), 0.f, 1.f );

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

	auto pos_comp = [] ( const Vertex& a, const Vertex& b ) { return a.position == b.position; };

	points_->update_last_point( v, pos_comp );

	if ( get_input()->press( Input::A ) )
	{
		if ( tablet_->get_cursor_index() == 1 )
		{
			points_->add_point( v, pos_comp );
		}
		else if ( tablet_->get_cursor_index() == 2 )
		{
			points_->erase_point( [this,&vector] ( const Vertex& a ) { Vector aa( a.position.x(), a.position.y(), a.position.z(), 0.f ); return ( vector - aa ).length() < tablet_->get_pressure() * 0.1f; } );
		}
	}
}

/**
 * •`‰æ
 *
 */
void CanvasTestScene::render()
{
	get_graphics_manager()->setup_rendering();
	get_graphics_manager()->render_background();

	get_graphics_manager()->set_input_layout( "drawing_point" );

	render_technique( "|drawing_point", [this] {
		get_graphics_manager()->get_game_render_data()->bind_to_all();
	
		get_graphics_manager()->get_frame_render_data()->bind_to_all();

		get_graphics_manager()->get_shared_object_render_data()->bind_to_vs();
		get_graphics_manager()->get_shared_object_render_data()->bind_to_ps();
		
		get_graphics_manager()->get_shared_object_render_data()->update( &object_constant_buffer_data );

		texture_->bind_to_ps( 1 );
		points_->render();
	} );

	{

		std::stringstream ss;
		ss.setf( std::ios_base::fixed, std::ios_base::floatfield );

		ss << "Time : " << get_total_elapsed_time() << '\n';
		ss << "FPS : " << get_main_loop()->get_last_fps() << '\n';
		ss << "C : " << tablet_->get_cursor_index() << '\n';
		ss << "X : " << tablet_->get_x() << '\n';
		ss << "Y : " << tablet_->get_y() << '\n';
		ss << "P : " << tablet_->get_pressure() << '\n';
		ss << "AZ : " << tablet_->get_azimuth() << '\n';
		ss << "AL : " << tablet_->get_altitude() << '\n';
		ss << "POINTS : " << points_->size() << '\n';

		get_graphics_manager()->draw_text( 10.f, 10.f, get_width() - 10.f, get_height() - 10.f, ss.str().c_str(), Color::Black );
	}
}

void CanvasTestScene::on_function_key_down( int key )
{
	if ( key == 8 )
	{
		points_->clear();
	}

	if ( key == 9 )
	{
		const std::array< char_t*, 7 > texture_names = {
			"media/texture/pen/white-soft-pen.png",
			"media/texture/pen/white-hard-pen.png",
			"media/texture/pen/bump-hard-pen.png",
			"media/texture/pen/bump-cross-pen.png",
			"media/texture/pen/white-grass-pen.png",
			"media/texture/pen/white-hard-round-rect-pen.png",
			"media/texture/pen/white-hard-round-rect-stroke-pen.png"
		};

		static int i = 0;
		i = ( i + 1 ) % texture_names.size();;

		texture_ = get_graphics_manager()->load_texture( texture_names[ i ] );
	}

	if ( key == 6 )
	{
		pen_mode_ = ( pen_mode_ + 1 ) % 3;
	}
}

} // namespace blue_sky
