#include "OptionsScene.h"

#include <blue_sky/GameMain.h>
#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/Input.h>
#include <blue_sky/App.h>

#include <core/sound/SoundManager.h>
#include <core/graphics/TextStyle.h>

#include <game/Config.h>

#include <sstream>
#include <iomanip>

namespace blue_sky
{

OptionsScene::OptionsScene()
	: ui_renderer_( get_graphics_manager() )
	, resolutions_( get_graphics_manager()->get_available_display_modes() )
	, is_mute_( get_sound_manager()->is_mute() )
	, is_fullscreen_( get_graphics_manager()->is_full_screen() )
{
	setup_widgets();
}

OptionsScene::~OptionsScene()
{
	save_settings();
}

int OptionsScene::find_current_resolution() const
{
	int w = GameMain::get_app()->get_width();
	int h = GameMain::get_app()->get_height();

	for ( int i = 0; i < static_cast< int >( resolutions_.size() ); i++ )
	{
		if ( resolutions_[ i ].width == w && resolutions_[ i ].height == h )
		{
			return i;
		}
	}

	return 0;
}

void OptionsScene::apply_resolution( int index )
{
	if ( index >= 0 && index < static_cast< int >( resolutions_.size() ) )
	{
		const auto& res = resolutions_[ index ];
		GameMain::get_app()->set_size( res.width, res.height );
	}
}

void OptionsScene::setup_widgets()
{
	// Resolution
	resolution_select_.set_label( "Resolution" );

	for ( const auto& res : resolutions_ )
	{
		std::stringstream ss;
		ss << res.width << " x " << res.height;
		resolution_select_.add_option( ss.str() );
	}

	resolution_select_.set_selected( find_current_resolution() );
	resolution_select_.set_on_change( [this] ( int index )
	{
		apply_resolution( index );
		play_sound( "ok" );
	} );

	// Volume
	float_t volume = get_sound_manager()->get_volume();
	volume_slider_.set_label( "Volume" );
	volume_slider_.set_range( 0.f, 1.f );
	volume_slider_.set_step( 0.1f );
	volume_slider_.set_value( volume );
	volume_slider_.set_format( [] ( float_t v ) -> string_t
	{
		int percent = static_cast< int >( v * 100.f + 0.5f );
		return std::to_string( percent ) + "%";
	} );
	volume_slider_.set_on_change( [this] ( float_t v )
	{
		get_sound_manager()->set_volume( v );
	} );

	// Mute
	mute_button_.set_text( is_mute_ ? "Mute : ON" : "Mute : OFF" );
	mute_button_.set_hint_text( "Enter/Click Toggle    ESC Back" );
	mute_button_.set_on_click( [this] ()
	{
		is_mute_ = ! is_mute_;
		get_sound_manager()->set_mute( is_mute_ );
		mute_button_.set_text( is_mute_ ? "Mute : ON" : "Mute : OFF" );
		play_sound( "ok" );
	} );

	// Mouse Sensitivity
	float_t sens = get_config()->get( "input.mouse.x_sensitivity", 1.f );
	mouse_sens_slider_.set_label( "Mouse Sens." );
	mouse_sens_slider_.set_range( 0.1f, 3.f );
	mouse_sens_slider_.set_step( 0.1f );
	mouse_sens_slider_.set_value( sens );
	mouse_sens_slider_.set_on_change( [this] ( float_t v )
	{
		get_input()->set_mouse_x_sensitivity( v );
		get_input()->set_mouse_y_sensitivity( v );
	} );

	// FOV
	float_t fov = get_config()->get( "camera.fov", 90.f );
	fov_slider_.set_label( "FOV" );
	fov_slider_.set_range( 50.f, 120.f );
	fov_slider_.set_step( 5.f );
	fov_slider_.set_value( fov );
	fov_slider_.set_format( [] ( float_t v ) -> string_t
	{
		return std::to_string( static_cast< int >( v + 0.5f ) );
	} );

	// Fullscreen
	fullscreen_button_.set_text( is_fullscreen_ ? "Fullscreen : ON" : "Fullscreen : OFF" );
	fullscreen_button_.set_hint_text( "Enter/Click Toggle    ESC Back" );
	fullscreen_button_.set_on_click( [this] ()
	{
		get_graphics_manager()->switch_full_screen();
		is_fullscreen_ = get_graphics_manager()->is_full_screen();
		fullscreen_button_.set_text( is_fullscreen_ ? "Fullscreen : ON" : "Fullscreen : OFF" );
		play_sound( "ok" );
	} );

	// Back
	back_button_.set_text( "Back" );
	back_button_.set_on_click( [this] ()
	{
		set_next_scene( "pop" );
		play_sound( "ok" );
	} );

	// コンテナに登録
	container_.add_widget( &resolution_select_ );
	container_.add_widget( &volume_slider_ );
	container_.add_widget( &mute_button_ );
	container_.add_widget( &mouse_sens_slider_ );
	container_.add_widget( &fov_slider_ );
	container_.add_widget( &fullscreen_button_ );
	container_.add_widget( &back_button_ );

	float_t screen_w = ui_renderer_.get_screen_width();
	float_t content_w = get_content_width();

	container_.set_position( ( screen_w - content_w ) * 0.5f, 200.f );
	container_.set_width( content_w );
	container_.set_item_height( 100.f );
	container_.layout();
}

void OptionsScene::save_settings()
{
	int res_index = resolution_select_.get_selected();

	if ( res_index >= 0 && res_index < static_cast< int >( resolutions_.size() ) )
	{
		const auto& res = resolutions_[ res_index ];
		get_config()->set( "graphics.screen_width", res.width );
		get_config()->set( "graphics.screen_height", res.height );
	}

	get_config()->set( "audio.volume", volume_slider_.get_value() );
	get_config()->set< int >( "audio.mute", is_mute_ ? 1 : 0 );
	get_config()->set( "input.mouse.x_sensitivity", mouse_sens_slider_.get_value() );
	get_config()->set( "input.mouse.y_sensitivity", mouse_sens_slider_.get_value() );
	get_config()->set( "camera.fov", fov_slider_.get_value() );
	get_config()->set< int >( "graphics.full_screen", is_fullscreen_ ? 1 : 0 );

	get_config()->save_file( "blue-sky.config" );
}

void OptionsScene::update()
{
	Scene::update();

	container_.update( get_input(), ui_renderer_ );
}

void OptionsScene::render()
{
	float_t screen_w = ui_renderer_.get_screen_width();
	float_t screen_h = ui_renderer_.get_screen_height();

	// 半透明の暗幕（全画面）
	ui_renderer_.draw_rect( 0.f, 0.f, screen_w, screen_h, Color( 0.f, 0.f, 0.f, 0.5f ) );

	// パネル背景
	float_t panel_w = get_panel_width();
	float_t panel_x = ( screen_w - panel_w ) * 0.5f;
	float_t panel_y = screen_h * 0.05f;
	float_t panel_h = screen_h * 0.90f;

	ui_renderer_.draw_rect( panel_x, panel_y, panel_w, panel_h, Color( 0.05f, 0.05f, 0.1f, 0.85f ) );

	// タイトル
	float_t content_w = get_content_width();
	float_t title_x = ( screen_w - content_w ) * 0.5f;
	float_t title_y = screen_h * 0.08f;
	ui_renderer_.draw_text( title_x, title_y, content_w, 120.f, "OPTIONS", core::graphics::TextStyle{ Color( 1.f, 1.f, 1.f, 1.f ), Color::Black, 3.f } );

	// ウィジェット描画
	container_.render( ui_renderer_ );

	// 操作ヒント
	float_t hint_y = screen_h * 0.90f;
	float_t hint_x = ( screen_w - content_w ) * 0.5f;

	const auto* focused = container_.get_focused_widget();
	const char_t* hint = "ESC Back";

	if ( focused && ! focused->get_hint_text().empty() )
	{
		hint = focused->get_hint_text().c_str();
	}

	ui_renderer_.draw_text( hint_x, hint_y, content_w, 90.f, hint, core::graphics::TextStyle{ Color( 0.5f, 0.5f, 0.5f, 1.f ), Color::Black, 3.f } );
}

} // namespace blue_sky
