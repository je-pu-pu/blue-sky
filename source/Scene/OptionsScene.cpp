#include "OptionsScene.h"

#include <blue_sky/GameMain.h>
#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/Input.h>

#include <core/sound/SoundManager.h>

#include <common/serialize.h>

#include <sstream>

namespace blue_sky
{

OptionsScene::OptionsScene()
	: ui_renderer_( get_graphics_manager() )
	, bgm_volume_( get_sound_manager()->get_volume() )
	, se_volume_( 1.f )
{
	setup_menu();
}

OptionsScene::~OptionsScene()
{
}

void OptionsScene::setup_menu()
{
	bgm_volume_index_ = 0;
	menu_.add_item( "", nullptr );

	se_volume_index_ = 1;
	menu_.add_item( "", nullptr );

	menu_.add_item( "Back", [this] ()
	{
		set_next_scene( "pop" );
		play_sound( "ok" );
	} );

	menu_.set_width( 500.f );
	menu_.center_on_screen( ui_renderer_ );
	update_volume_text();
}

void OptionsScene::update_volume_text()
{
	{
		int percent = static_cast< int >( bgm_volume_ * 100.f + 0.5f );

		std::stringstream ss;
		ss << "BGM Volume  : " << percent << "%";

		menu_.set_item_text( bgm_volume_index_, ss.str() );
	}

	{
		int percent = static_cast< int >( se_volume_ * 100.f + 0.5f );

		std::stringstream ss;
		ss << "SE Volume   : " << percent << "%";

		menu_.set_item_text( se_volume_index_, ss.str() );
	}
}

void OptionsScene::adjust_bgm_volume( float_t delta )
{
	bgm_volume_ += delta;

	if ( bgm_volume_ < 0.f ) bgm_volume_ = 0.f;
	if ( bgm_volume_ > 1.f ) bgm_volume_ = 1.f;

	get_sound_manager()->set_volume( bgm_volume_ );
	update_volume_text();
}

void OptionsScene::adjust_se_volume( float_t delta )
{
	se_volume_ += delta;

	if ( se_volume_ < 0.f ) se_volume_ = 0.f;
	if ( se_volume_ > 1.f ) se_volume_ = 1.f;

	/// @todo SE 専用のボリューム制御を実装する
	update_volume_text();
}

void OptionsScene::update()
{
	Scene::update();

	// 左右キーでボリューム調整
	if ( menu_.get_selected_index() == bgm_volume_index_ )
	{
		if ( get_input()->push( Input::Button::LEFT ) )
		{
			adjust_bgm_volume( -get_volume_step() );
		}
		if ( get_input()->push( Input::Button::RIGHT ) )
		{
			adjust_bgm_volume( +get_volume_step() );
		}
	}
	else if ( menu_.get_selected_index() == se_volume_index_ )
	{
		if ( get_input()->push( Input::Button::LEFT ) )
		{
			adjust_se_volume( -get_volume_step() );
		}
		if ( get_input()->push( Input::Button::RIGHT ) )
		{
			adjust_se_volume( +get_volume_step() );
		}
	}

	menu_.update( get_input() );
}

void OptionsScene::render()
{
	// 半透明の暗幕を描画
	get_graphics_manager()->set_fade_color( Color( 0.f, 0.f, 0.f, 0.7f ) );
	render_fader();

	// タイトル
	float_t title_x = ( static_cast< float_t >( ui_renderer_.get_screen_width() ) - 500.f ) * 0.5f;
	float_t title_y = static_cast< float_t >( ui_renderer_.get_screen_height() ) * 0.15f;
	ui_renderer_.draw_text( title_x, title_y, 500.f, 80.f, "OPTIONS", Color( 1.f, 1.f, 1.f, 1.f ) );

	// メニュー
	menu_.render( ui_renderer_ );
}

} // namespace blue_sky
