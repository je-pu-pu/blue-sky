#include "TitleScene.h"

#include "GameMain.h"
#include "ConstantBuffer.h"

#include "DrawingModelManager.h"
#include "DrawingModel.h"
#include "DrawingLine.h"

#include "Input.h"
#include "GraphicsManager.h"
#include "SoundManager.h"
#include "Sound.h"

#include "Direct3D11.h"
#include "Direct3D11Sprite.h"
#include "Direct3D11Effect.h"
#include "Direct3D11Fader.h"
#include "Direct3D11ConstantBuffer.h"
#include "Direct3D11TextureManager.h"
#include "Direct3D11Color.h"
#include "DirectX.h"

#include <win/Rect.h>

#include <common/math.h>
#include <common/serialize.h>
#include <common/exception.h>

#include "memory.h"

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
	
	// title_texture_ = direct_3d()->getTextureManager()->load( "sprite", "media/texture/title.png" );
	title_bg_texture_ = get_direct_3d()->getTextureManager()->load( "title-bg", "media/image/cloud-3.jpg" );
	cloth_texture_ = get_direct_3d()->getTextureManager()->load( "cloth", "media/texture/cloth.png" );

	get_graphics_manager()->setup_loader();

	brand_logo_model_ = get_drawing_model_manager()->load( "je-pu-pu" );
	title_logo_model_ = get_drawing_model_manager()->load( "blue-sky" );

	get_graphics_manager()->cleanup_loader();

	{
		GameConstantBufferData game_constant_buffer_data;
		game_constant_buffer_data.screen_width = static_cast< float_t >( get_width() );
		game_constant_buffer_data.screen_height = static_cast< float_t >( get_height() );
		
		get_game_main()->get_game_constant_buffer()->update( & game_constant_buffer_data );
	}
	
	reset_total_elapsed_time();
	bgm_->play( false );

	get_direct_3d()->getFader()->full_out();
}

TitleScene::~TitleScene()
{
	
}

/**
 * ƒƒCƒ“ƒ‹[ƒvˆ—
 *
 */
void TitleScene::update()
{
	Scene::update();

	sequence_elapsed_time_ += get_elapsed_time();
	get_direct_3d()->getFader()->fade_in();

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
			get_direct_3d()->getFader()->full_out();
		}
	}

	if ( bgm_->get_current_position() >= 8.25f )
	{
		if ( sequence_ < SEQUENCE_TITLE_FIX )
		{
			sequence_ = SEQUENCE_TITLE_FIX;
			sequence_elapsed_time_ = 0.f;
			get_direct_3d()->getFader()->full_out();
		}
	}
	else if ( bgm_->get_current_position() >= 4.25f )
	{
		if ( sequence_ < SEQUENCE_TITLE_LOGO )
		{
			sequence_ = SEQUENCE_TITLE_LOGO;
			sequence_elapsed_time_ = 0.f;
			get_direct_3d()->getFader()->full_out();
		}
	}
}

/**
 * •`‰æ
 */
void TitleScene::render()
{
	update_constant_buffer_for_sprite_frame( 0 );

	get_direct_3d()->clear( Direct3D::Color::from_256( 0xFF, 0xAA, 0x11 ) );
	get_direct_3d()->getSprite()->begin();

	Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( "|sprite" );

	for ( Direct3D::EffectTechnique::PassList::const_iterator i = technique->getPassList().begin(); i != technique->getPassList().end(); ++i )
	{
		( *i )->apply();

		win::Rect dst_rect( 0, 0, get_width(), get_height() );

		if ( sequence_ >= SEQUENCE_TITLE_FIX )
		{
			// render_bg()
			get_direct_3d()->getSprite()->draw( dst_rect, title_bg_texture_ );
		}
		else
		{
			// render_bg()
			get_direct_3d()->getSprite()->draw( dst_rect, cloth_texture_, Direct3D::Color( 1.f, 1.f, 1.f, 0.5f ) );
		}
	}

	get_direct_3d()->getSprite()->end();

	// render_logo()
	{
		get_direct_3d()->setInputLayout( "line" );

		Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( "|drawing_line" );

		for ( Direct3D::EffectTechnique::PassList::const_iterator i = technique->getPassList().begin(); i != technique->getPassList().end(); ++i )
		{
			( *i )->apply();

			get_game_main()->get_game_constant_buffer()->bind_to_all(); /// !!!
			get_game_main()->get_frame_constant_buffer()->bind_to_all(); /// !!!
			get_game_main()->get_frame_drawing_constant_buffer()->bind_to_all(); /// !!!
			get_game_main()->get_object_constant_buffer()->bind_to_all(); /// !!!

			if ( sequence_ == SEQUENCE_LOGO )
			{
				brand_logo_model_->get_line()->render_part( static_cast< int >( sequence_elapsed_time_ * 25 ) );
			}
			else if ( sequence_ >= SEQUENCE_TITLE_LOGO )
			{
				if ( sequence_ >= SEQUENCE_TITLE_FIX )
				{
					{
						ObjectConstantBufferData object_constant_buffer_data;

						object_constant_buffer_data.world = XMMatrixTranspose( XMMatrixTranslation( +0.01f, +0.01f, 0.f ) );
						object_constant_buffer_data.color = Color( 0.f, 0.f, 0.f, -0.5f );

						get_game_main()->get_object_constant_buffer()->update( & object_constant_buffer_data );
					}

					title_logo_model_->get_line()->render();

					{
						ObjectConstantBufferData object_constant_buffer_data;

						object_constant_buffer_data.world = XMMatrixTranspose( XMMatrixIdentity() );
						object_constant_buffer_data.color = Color( 1.f, 1.f, 1.f, 0.f );

						get_game_main()->get_object_constant_buffer()->update( & object_constant_buffer_data );
					}

					title_logo_model_->get_line()->render();
				}
				else
				{
					title_logo_model_->get_line()->render_part( static_cast< int >( sequence_elapsed_time_ * 25 ) );
				}
			}
		}
	}

	render_fader();
}

} // namespace blue_sky