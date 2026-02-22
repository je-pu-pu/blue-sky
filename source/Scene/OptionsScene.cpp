#include "OptionsScene.h"

#include <blue_sky/GameMain.h>
#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/Input.h>
#include <blue_sky/App.h>

#include <core/sound/SoundManager.h>

#include <game/Config.h>

#include <sstream>

namespace blue_sky
{

OptionsScene::OptionsScene()
	: ui_renderer_( get_graphics_manager() )
	, resolutions_( get_graphics_manager()->get_available_display_modes() )
	, volume_( get_sound_manager()->get_volume() )
	, is_mute_( get_sound_manager()->is_mute() )
	, mouse_sensitivity_( get_config()->get( "input.mouse.x_sensitivity", 1.f ) )
	, fov_( get_config()->get( "camera.fov", 90.f ) )
	, is_fullscreen_( get_graphics_manager()->is_full_screen() )
{
	current_resolution_ = find_current_resolution();
	setup_menu();
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

void OptionsScene::apply_resolution()
{
	const auto& res = resolutions_[ current_resolution_ ];
	GameMain::get_app()->set_size( res.width, res.height );
}

void OptionsScene::setup_menu()
{
	int index = 0;

	// Resolution
	resolution_index_ = index++;
	menu_.add_item( "", nullptr );

	// Volume
	volume_index_ = index++;
	menu_.add_item( "", nullptr );

	// Mute
	mute_index_ = index++;
	menu_.add_item( "", [this] ()
	{
		is_mute_ = ! is_mute_;
		get_sound_manager()->set_mute( is_mute_ );
		update_all_text();
		play_sound( "ok" );
	} );

	// Mouse Sensitivity
	mouse_sens_index_ = index++;
	menu_.add_item( "", nullptr );

	// FOV
	fov_index_ = index++;
	menu_.add_item( "", nullptr );

	// Fullscreen
	fullscreen_index_ = index++;
	menu_.add_item( "", [this] ()
	{
		get_graphics_manager()->switch_full_screen();
		is_fullscreen_ = get_graphics_manager()->is_full_screen();
		update_all_text();
		play_sound( "ok" );
	} );

	// Back
	back_index_ = index++;
	menu_.add_item( "Back", [this] ()
	{
		set_next_scene( "pop" );
		play_sound( "ok" );
	} );

	float_t screen_w = ui_renderer_.get_screen_width();
	float_t menu_w = get_content_width();

	menu_.set_width( menu_w );
	menu_.set_item_height( 100.f );
	menu_.set_position( ( screen_w - menu_w ) * 0.5f, 200.f );
	update_all_text();
}

string_t OptionsScene::make_bar( float_t value, float_t min_val, float_t max_val, int bar_width )
{
	float_t ratio = ( value - min_val ) / ( max_val - min_val );

	if ( ratio < 0.f ) ratio = 0.f;
	if ( ratio > 1.f ) ratio = 1.f;

	int filled = static_cast< int >( ratio * bar_width + 0.5f );

	string_t bar = "[";

	for ( int i = 0; i < bar_width; i++ )
	{
		bar += ( i < filled ) ? "=" : " ";
	}

	bar += "]";

	return bar;
}

void OptionsScene::update_all_text()
{
	// Resolution
	{
		const auto& res = resolutions_[ current_resolution_ ];
		std::stringstream ss;
		ss << "Resolution    < " << res.width << " x " << res.height << " >";
		menu_.set_item_text( resolution_index_, ss.str() );
	}

	// Volume
	{
		int percent = static_cast< int >( volume_ * 100.f + 0.5f );
		std::stringstream ss;
		ss << "Volume        " << make_bar( volume_, 0.f, 1.f ) << " " << percent << "%";
		menu_.set_item_text( volume_index_, ss.str() );
	}

	// Mute
	{
		std::stringstream ss;
		ss << "Mute          " << ( is_mute_ ? "ON" : "OFF" );
		menu_.set_item_text( mute_index_, ss.str() );
	}

	// Mouse Sensitivity
	{
		std::stringstream ss;
		ss << "Mouse Sens.   " << make_bar( mouse_sensitivity_, get_min_sensitivity(), get_max_sensitivity() );
		ss << " " << std::fixed;
		ss.precision( 1 );
		ss << mouse_sensitivity_;
		menu_.set_item_text( mouse_sens_index_, ss.str() );
	}

	// FOV
	{
		int fov_int = static_cast< int >( fov_ + 0.5f );
		std::stringstream ss;
		ss << "FOV           " << make_bar( fov_, get_min_fov(), get_max_fov() ) << " " << fov_int;
		menu_.set_item_text( fov_index_, ss.str() );
	}

	// Fullscreen
	{
		std::stringstream ss;
		ss << "Fullscreen    " << ( is_fullscreen_ ? "ON" : "OFF" );
		menu_.set_item_text( fullscreen_index_, ss.str() );
	}
}

void OptionsScene::adjust_value( float_t& value, float_t delta, float_t min_val, float_t max_val )
{
	value += delta;

	if ( value < min_val ) value = min_val;
	if ( value > max_val ) value = max_val;
}

void OptionsScene::save_settings()
{
	const auto& res = resolutions_[ current_resolution_ ];
	get_config()->set( "graphics.screen_width", res.width );
	get_config()->set( "graphics.screen_height", res.height );

	get_config()->set( "audio.volume", volume_ );
	get_config()->set< int >( "audio.mute", is_mute_ ? 1 : 0 );
	get_config()->set( "input.mouse.x_sensitivity", mouse_sensitivity_ );
	get_config()->set( "input.mouse.y_sensitivity", mouse_sensitivity_ );
	get_config()->set( "camera.fov", fov_ );
	get_config()->set< int >( "graphics.full_screen", is_fullscreen_ ? 1 : 0 );

	get_config()->save_file( "blue-sky.config" );
}

void OptionsScene::update()
{
	Scene::update();

	int selected = menu_.get_selected_index();

	// 左右キーでスライダー / 選択項目を調整
	bool is_left = get_input()->push( Input::Button::LEFT );
	bool is_right = get_input()->push( Input::Button::RIGHT );

	if ( selected == resolution_index_ && ( is_left || is_right ) )
	{
		if ( is_right && current_resolution_ < static_cast< int >( resolutions_.size() ) - 1 )
		{
			current_resolution_++;
		}
		else if ( is_left && current_resolution_ > 0 )
		{
			current_resolution_--;
		}

		apply_resolution();
		update_all_text();
	}
	else if ( selected == volume_index_ && ( is_left || is_right ) )
	{
		float_t delta = is_right ? get_volume_step() : -get_volume_step();
		adjust_value( volume_, delta, 0.f, 1.f );
		get_sound_manager()->set_volume( volume_ );
		update_all_text();
	}
	else if ( selected == mouse_sens_index_ && ( is_left || is_right ) )
	{
		float_t delta = is_right ? get_sensitivity_step() : -get_sensitivity_step();
		adjust_value( mouse_sensitivity_, delta, get_min_sensitivity(), get_max_sensitivity() );
		get_input()->set_mouse_x_sensitivity( mouse_sensitivity_ );
		get_input()->set_mouse_y_sensitivity( mouse_sensitivity_ );
		update_all_text();
	}
	else if ( selected == fov_index_ && ( is_left || is_right ) )
	{
		float_t delta = is_right ? get_fov_step() : -get_fov_step();
		adjust_value( fov_, delta, get_min_fov(), get_max_fov() );
		update_all_text();
	}

	menu_.update( get_input(), ui_renderer_ );
}

void OptionsScene::render()
{
	float_t screen_w = ui_renderer_.get_screen_width();
	float_t screen_h = ui_renderer_.get_screen_height();

	// 半透明の暗幕（全画面）— Fader は fade_==0 時に透明になるため draw_rect を使用
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
	ui_renderer_.draw_text( title_x, title_y, content_w, 120.f, "OPTIONS", Color( 1.f, 1.f, 1.f, 1.f ), Color::Black, 0.15f );

	// メニュー
	menu_.render( ui_renderer_ );

	// 操作ヒント
	float_t hint_y = screen_h * 0.90f;
	float_t hint_x = ( screen_w - content_w ) * 0.5f;

	int selected = menu_.get_selected_index();

	if ( selected == resolution_index_ )
	{
		ui_renderer_.draw_text( hint_x, hint_y, content_w, 90.f, "Arrow/AD Change    ESC Back", Color( 0.5f, 0.5f, 0.5f, 1.f ), Color::Black, 0.15f );
	}
	else if ( selected == volume_index_ || selected == mouse_sens_index_ || selected == fov_index_ )
	{
		ui_renderer_.draw_text( hint_x, hint_y, content_w, 90.f, "Arrow/AD Adjust    ESC Back", Color( 0.5f, 0.5f, 0.5f, 1.f ), Color::Black, 0.15f );
	}
	else if ( selected == mute_index_ || selected == fullscreen_index_ )
	{
		ui_renderer_.draw_text( hint_x, hint_y, content_w, 90.f, "Enter/Click Toggle    ESC Back", Color( 0.5f, 0.5f, 0.5f, 1.f ), Color::Black, 0.15f );
	}
	else
	{
		ui_renderer_.draw_text( hint_x, hint_y, content_w, 90.f, "Enter/Click Select    ESC Back", Color( 0.5f, 0.5f, 0.5f, 1.f ), Color::Black, 0.15f );
	}
}

} // namespace blue_sky
