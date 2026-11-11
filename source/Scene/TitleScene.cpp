#include "TitleScene.h"

#include <blue_sky/GameMain.h>
#include <blue_sky/Input.h>
#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/graphics/Model.h>
#include <blue_sky/graphics/Line.h>
#include <blue_sky/graphics/Fader.h>

#include <core/sound/SoundManager.h>
#include <core/sound/Sound.h>
#include <core/graphics/Sprite.h>

#include <win/Rect.h>

#include <common/math.h>
#include <common/serialize.h>
#include <common/exception.h>

namespace blue_sky
{

TitleScene::TitleScene( GameMain& game_main )
	: Scene( game_main )
	, sequence_( 0 )
	, sequence_elapsed_time_( 0.f )
{
	ok_ = get_sound_manager()->get_sound_handle( "ok" );
	get_sound_manager()->load_music( "bgm", "ending" );
	bgm_ = get_sound_manager()->get_sound_handle( "bgm" );

	title_bg_texture_ = load_named_texture_handle( "title-bg", "media/image/cloud-3.jpg" );
	cloth_texture_ = load_named_texture_handle( "cloth", "media/texture/cloth.png" );

	brand_logo_model_ = load_model_handle( "je-pu-pu" );
	title_logo_model_ = load_model_handle( "blue-sky" );

	reset_total_elapsed_time();
	play( bgm_, "bgm" );

	get_graphics_manager()->get_fader()->full_out();
}

TitleScene::~TitleScene()
{
	
}

/**
 * メインループ処理
 *
 */
void TitleScene::update()
{
	Scene::update();

	sequence_elapsed_time_ += get_elapsed_time();
	get_graphics_manager()->get_fader()->fade_in();

	if ( get_input()->push( Input::Button::A ) )
	{
		if ( sequence_ == SEQUENCE_TITLE_FIX )
		{
			play( ok_, "ok" );
			set_next_scene( "stage_select" );
		}
		else if ( ! is_first_game_play() )
		{
			play( ok_, "ok" );
			sequence_ = SEQUENCE_TITLE_FIX;
			sequence_elapsed_time_ = 0.f;
			get_graphics_manager()->get_fader()->full_out();
		}
	}

	// BGM が引けなければ、時間でロゴを進めない（ボタンでは進められる）
	const Sound* bgm = get_bgm();
	const float_t bgm_position = bgm ? bgm->get_current_position() : 0.f;

	if ( bgm_position >= 8.25f )
	{
		if ( sequence_ < SEQUENCE_TITLE_FIX )
		{
			sequence_ = SEQUENCE_TITLE_FIX;
			sequence_elapsed_time_ = 0.f;
			get_graphics_manager()->get_fader()->full_out();
		}
	}
	else if ( bgm_position >= 4.25f )
	{
		if ( sequence_ < SEQUENCE_TITLE_LOGO )
		{
			sequence_ = SEQUENCE_TITLE_LOGO;
			sequence_elapsed_time_ = 0.f;
			get_graphics_manager()->get_fader()->full_out();
		}
	}
}

/**
 * 描画
 */
void TitleScene::render()
{
	update_constant_buffer_for_sprite_frame( 0 );

	get_graphics_manager()->set_default_render_target();
	get_graphics_manager()->set_default_viewport();

	get_graphics_manager()->clear_default_view( Color::from_256( 0xFF, 0xAA, 0x11 ) );

	auto* sprite = get_graphics_manager()->get_sprite();
	sprite->begin();

	{
		render_technique( "|sprite", [this, sprite]
		{
			win::Rect dst_rect( 0, 0, get_width(), get_height() );

			if ( sequence_ >= SEQUENCE_TITLE_FIX )
			{
				// render_bg()
				sprite->draw( dst_rect, resolve( title_bg_texture_, "title-bg" ) );	// 引けなければ Sprite は描かない
			}
			else
			{
				// render_bg()
				sprite->draw( dst_rect, resolve( cloth_texture_, "cloth" ), Color( 1.f, 1.f, 1.f, 0.5f ) );
			}
		} );
	}

	sprite->end();

	// render_logo()
	{
		get_graphics_manager()->set_input_layout( "pos_color" );

		render_technique( "|drawing_line", [this]
		{
			bind_all_render_data();

			// モデルは引けなければ描かない
			if ( sequence_ == SEQUENCE_LOGO )
			{
				if ( const Model* brand_logo = resolve( brand_logo_model_, "je-pu-pu" ) )
				{
					brand_logo->get_line()->render_part( static_cast< int >( sequence_elapsed_time_ * 25 ) );
				}
			}
			else if ( const Model* title_logo = resolve( title_logo_model_, "blue-sky" ); title_logo && sequence_ >= SEQUENCE_TITLE_LOGO )
			{
				if ( sequence_ >= SEQUENCE_TITLE_FIX )
				{
					{
						ObjectConstantBufferData object_constant_buffer_data;

						object_constant_buffer_data.world = Matrix().set_translation( +0.01f, +0.01f, 0.f );
						object_constant_buffer_data.color = Color( 0.f, 0.f, 0.f, -0.5f );

						get_graphics_manager()->get_shared_object_render_data()->update( & object_constant_buffer_data );
					}

					title_logo->get_line()->render();

					{
						ObjectConstantBufferData object_constant_buffer_data;

						object_constant_buffer_data.world = Matrix().set_identity();
						object_constant_buffer_data.color = Color( 1.f, 1.f, 1.f, 0.f );

						get_graphics_manager()->get_shared_object_render_data()->update( & object_constant_buffer_data );
					}

					title_logo->get_line()->render();
				}
				else
				{
					title_logo->get_line()->render_part( static_cast< int >( sequence_elapsed_time_ * 25 ) );
				}
			}
		} );
	}

	render_fader();
}

} // namespace blue_sky