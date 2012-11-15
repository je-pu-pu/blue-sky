#include "TitleScene.h"

#include "GameMain.h"
#include "ConstantBuffer.h"

#include "DrawingModelManager.h"
#include "DrawingModel.h"
#include "DrawingLine.h"

#include "Input.h"
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
	ok_ = get_sound_manager()->load( "ok" );
	bgm_ = get_sound_manager()->load_music( "bgm", "title" );
	
	// title_texture_ = direct_3d()->getTextureManager()->load( "sprite", "media/texture/title.png" );
	title_bg_texture_ = get_direct_3d()->getTextureManager()->load( "title-bg", "media/image/cloud-3.jpg" );
	cloth_texture_ = get_direct_3d()->getTextureManager()->load( "cloth", "media/texture/cloth.png" );

	brand_logo_model_ = get_drawing_model_manager()->load( "je-pu-pu" );
	title_logo_model_ = get_drawing_model_manager()->load( "blue-sky" );

	{
		GameConstantBufferData game_constant_buffer_data;

		game_constant_buffer_data.projection = XMMatrixTranspose( XMMatrixOrthographicLH( 2.f * get_width() / get_height(), 2.f, 0.f, 1.f ) );
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
	{
		FrameConstantBufferData frame_constant_buffer_data;

		frame_constant_buffer_data.view = XMMatrixTranspose( XMMatrixIdentity() );
		frame_constant_buffer_data.time = get_total_elapsed_time();
	
		get_game_main()->get_frame_constant_buffer()->update( & frame_constant_buffer_data );
	}

	{
		ObjectConstantBufferData object_constant_buffer_data;

		object_constant_buffer_data.world = XMMatrixTranspose( XMMatrixIdentity() );

		get_game_main()->get_object_constant_buffer()->update( & object_constant_buffer_data );
	}

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
			get_game_main()->get_object_constant_buffer()->bind_to_all(); /// !!!

			if ( sequence_ == SEQUENCE_LOGO )
			{
				brand_logo_model_->get_line()->render( static_cast< int >( sequence_elapsed_time_ * 50 ) );
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
					title_logo_model_->get_line()->render( static_cast< int >( sequence_elapsed_time_ * 50 ) );
				}
			}
		}
	}

	render_fader();

#if 0
	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloat( "brightness", 1.f ) );

	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0xFF, 0xAA, 0x11 ), 1.f, 0 ) );

	DIRECT_X_FAIL_CHECK( direct_3d()->getSprite()->Begin( D3DXSPRITE_ALPHABLEND ) );

	UINT pass_count = 0;
	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->Begin( & pass_count, 0 ) );
	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->BeginPass( 0 ) );

	D3DXMATRIXA16 transform;
	D3DXMATRIXA16 s, t;

	// BG
	{
		win::Rect bg_src_rect( 0, 1024, 2048, 2048 );
		D3DXVECTOR3 bg_center( bg_src_rect.width() * 0.5f, bg_src_rect.height() * 0.5f, 0.f );

		float ratio = static_cast< float >( get_height() ) / static_cast< float >( bg_src_rect.height() );

		D3DXMatrixScaling( & s, ratio, ratio, 1.f );
		D3DXMatrixTranslation( & t, get_width() * 0.5f, get_height() * 0.5f, 0.f );

		transform = s * t;
		direct_3d()->getSprite()->SetTransform( & transform );
		direct_3d()->getSprite()->Draw(  title_bg_texture_, & bg_src_rect.get_rect(), & bg_center, 0, D3DCOLOR_ARGB( 0xFF, 0xFF, 0x66, 0x11 ) );
	}

	// Logo
	if ( sequence_ == SEQUENCE_LOGO )
	{
		RECT src_rect = { 0, 0, 400, 128 };
		D3DXVECTOR3 center( ( src_rect.right - src_rect.left ) * 0.5f, ( src_rect.bottom - src_rect.top ) * 0.5f, 0.f );
		
		D3DXMatrixTranslation( & t, get_width() * 0.5f, get_height() * 0.5f, 0.f );

		transform = t;
		direct_3d()->getSprite()->SetTransform( & transform );
		direct_3d()->getSprite()->Draw( title_texture_, & src_rect, & center, 0, 0xFFFFFFFF );
	}

	// Title BG
	if ( sequence_ >= SEQUENCE_TITLE_BG )
	{
		RECT src_rect = { 0, 0, 2048, 1024 };
		D3DXVECTOR3 center( ( src_rect.right - src_rect.left ) * 0.5f, ( src_rect.bottom - src_rect.top ) * 0.5f, 0.f );
		D3DXVECTOR3 position( get_width() * 0.5f, get_height() * 0.5f, 0.f );

		D3DXMatrixTranslation( & t, get_width() * 0.5f, get_height() * 0.5f, 0.f );

		float ratio = static_cast< float >( get_height() ) / static_cast< float >( src_rect.bottom - src_rect.top );
		D3DXMatrixScaling( & s, ratio * title_bg_scale_, ratio * title_bg_scale_, 1.f );

		transform = s * t;
		direct_3d()->getSprite()->SetTransform( & transform );
		direct_3d()->getSprite()->Draw(  title_bg_texture_, & src_rect, & center, 0, 0xFFFFFFFF );
	}

	// Title Logo
	if ( sequence_ >= SEQUENCE_TITLE_LOGO )
	{
		RECT src_rect = { 0, 128, 540, 448 };
		D3DXVECTOR3 center( ( src_rect.right - src_rect.left ) * 0.5f, ( src_rect.bottom - src_rect.top ) * 0.5f, 0.f );
		D3DXVECTOR3 position( get_width() * 0.5f, get_height() * 0.5f, 0.f );

		D3DXMatrixIdentity( & transform );
		direct_3d()->getSprite()->SetTransform( & transform );
		direct_3d()->getSprite()->Draw( title_texture_, & src_rect, & center, & position, 0xFFFFFFFF );
	}

	// Fade
	if ( fade_alpha_ > 0.f )
	{
		RECT src_rect = { 0, 512, 128, 512 + 128 };
		D3DXVECTOR3 center( ( src_rect.right - src_rect.left ) * 0.5f, ( src_rect.bottom - src_rect.top ) * 0.5f, 0.f );
		
		float ratio = static_cast< float >( std::max( get_width(), get_height() ) ) / 128.f;
		D3DXMatrixScaling( & s, ratio, ratio, 1.f );
		D3DXMatrixTranslation( & t, get_width() * 0.5f, get_height() * 0.5f, 0.f );

		transform = s * t;
		direct_3d()->getSprite()->SetTransform( & transform );
		direct_3d()->getSprite()->Draw( title_texture_, & src_rect, & center, 0, D3DCOLOR_RGBA( 255, 255, 255, static_cast< int >( fade_alpha_ * 255 ) ) );
	}
	
	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->EndPass() );
	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->End() );

	DIRECT_X_FAIL_CHECK( direct_3d()->getSprite()->End() );

	/*
	std::string debug_text = "bgm pos : " +  common::serialize( bgm_->get_current_position() );
	direct_3d()->getFont()->draw_text( -1, 0, debug_text.c_str(), D3DCOLOR_XRGB( 255, 0, 0 ) );
	*/

	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->EndScene() );
#endif

}

} // namespace blue_sky