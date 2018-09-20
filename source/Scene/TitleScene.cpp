#include "TitleScene.h"

#include "GameMain.h"
// #include "ConstantBuffer.h"

#include "Input.h"

#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/graphics/Model.h>
#include <blue_sky/graphics/Line.h>
#include <blue_sky/graphics/Fader.h>

#include <core/sound/SoundManager.h>
#include <core/sound/Sound.h>

/// @todo íäè€âªÇ∑ÇÈ
#include <core/graphics/Direct3D11/Direct3D11.h>
#include <core/graphics/Direct3D11/Sprite.h>

#include <win/Rect.h>

#include <common/math.h>
#include <common/serialize.h>
#include <common/exception.h>

namespace blue_sky
{

TitleScene::TitleScene( const GameMain* game_main )
	: Scene( game_main )
	, title_texture_( 0 )
	, title_bg_texture_( 0 )
	, cloth_texture_( 0 )
	, brand_logo_model_( 0 )
	, title_logo_model_( 0 )
	, ok_( 0 )
	, bgm_( 0 )
	, sequence_( 0 )
	, sequence_elapsed_time_( 0.f )
{
	ok_ = get_sound_manager()->get_sound( "ok" );
	bgm_ = get_sound_manager()->load_music( "bgm", "ending" );
	
	// title_texture_ = get_graphics_manager()->load_named_texture( "sprite", "media/texture/title.png" );
	title_bg_texture_ = get_graphics_manager()->load_named_texture( "title-bg", "media/image/cloud-3.jpg" );
	cloth_texture_ = get_graphics_manager()->load_named_texture( "cloth", "media/texture/cloth.png" );

	brand_logo_model_ = get_graphics_manager()->load_model( "je-pu-pu" );
	title_logo_model_ = get_graphics_manager()->load_model( "blue-sky" );

	reset_total_elapsed_time();
	bgm_->play( false );

	get_graphics_manager()->get_fader()->full_out();
}

TitleScene::~TitleScene()
{
	
}

/**
 * ÉÅÉCÉìÉãÅ[Évèàóù
 *
 */
void TitleScene::update()
{
	Scene::update();

	sequence_elapsed_time_ += get_elapsed_time();
	get_graphics_manager()->get_fader()->fade_in();

	if ( get_input()->push( Input::A ) )
	{
		if ( sequence_ == SEQUENCE_TITLE_FIX )
		{
			ok_->play( false );
			set_next_scene( "stage_select" );
		}
		else if ( ! is_first_game_play() )
		{
			ok_->play( false );
			sequence_ = SEQUENCE_TITLE_FIX;
			sequence_elapsed_time_ = 0.f;
			get_graphics_manager()->get_fader()->full_out();
		}
	}

	if ( bgm_->get_current_position() >= 8.25f )
	{
		if ( sequence_ < SEQUENCE_TITLE_FIX )
		{
			sequence_ = SEQUENCE_TITLE_FIX;
			sequence_elapsed_time_ = 0.f;
			get_graphics_manager()->get_fader()->full_out();
		}
	}
	else if ( bgm_->get_current_position() >= 4.25f )
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
 * ï`âÊ
 */
void TitleScene::render()
{
	update_constant_buffer_for_sprite_frame( 0 );

	get_direct_3d()->set_default_render_target();
	get_direct_3d()->set_default_viewport();

	get_direct_3d()->clear_default_view( Direct3D::Color::from_256( 0xFF, 0xAA, 0x11 ) );

	get_direct_3d()->get_sprite()->begin();

	{
		render_technique( "|sprite", [this]
		{
			win::Rect dst_rect( 0, 0, get_width(), get_height() );

			if ( sequence_ >= SEQUENCE_TITLE_FIX )
			{
				// render_bg()
				get_direct_3d()->get_sprite()->draw( dst_rect, title_bg_texture_ );
			}
			else
			{
				// render_bg()
				get_direct_3d()->get_sprite()->draw( dst_rect, cloth_texture_, Direct3D::Color( 1.f, 1.f, 1.f, 0.5f ) );
			}
		} );
	}

	get_direct_3d()->get_sprite()->end();

	// render_logo()
	{
		get_graphics_manager()->set_input_layout( "line" );

		render_technique( "|drawing_line", [this]
		{
			bind_all_render_data();

			if ( sequence_ == SEQUENCE_LOGO )
			{
				brand_logo_model_->get_line()->render_part( static_cast< int >( sequence_elapsed_time_ * 25 ) );
			}
			else if ( sequence_ >= SEQUENCE_TITLE_LOGO )
			{
				if ( sequence_ >= SEQUENCE_TITLE_FIX )
				{
					{
						ObjectShaderResourceData object_constant_buffer_data;

						object_constant_buffer_data.world = Matrix().set_translation( +0.01f, +0.01f, 0.f ).transpose();
						object_constant_buffer_data.color = Color( 0.f, 0.f, 0.f, -0.5f );

						get_graphics_manager()->get_shared_object_render_data()->update( & object_constant_buffer_data );
					}

					title_logo_model_->get_line()->render();

					{
						ObjectShaderResourceData object_constant_buffer_data;

						object_constant_buffer_data.world = Matrix().set_identity().transpose();
						object_constant_buffer_data.color = Color( 1.f, 1.f, 1.f, 0.f );

						get_graphics_manager()->get_shared_object_render_data()->update( & object_constant_buffer_data );
					}

					title_logo_model_->get_line()->render();
				}
				else
				{
					title_logo_model_->get_line()->render_part( static_cast< int >( sequence_elapsed_time_ * 25 ) );
				}
			}
		} );
	}

	render_fader();
}

} // namespace blue_sky