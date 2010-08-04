#include "TitleScene.h"

#include "Input.h"
#include "SoundManager.h"
#include "Sound.h"

#include "Direct3D9.h"
#include "Direct3D9Font.h"
#include "Direct3D9TextureManager.h"
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
	, ok_( 0 )
	, bgm_( 0 )
	, sequence_( 0 )
	, title_bg_scale_( 2.f )
	, title_bg_scale_cycle_( 0.f )
	, fade_alpha_( 1.f )
{
	ok_ = sound_manager()->load( "ok" );

	bgm_ = sound_manager()->load_music( "bgm", "title" );
	bgm_->play( false );
	
	title_texture_ = direct_3d()->getTextureManager()->load( "sprite", "media/image/title.png" );
	title_bg_texture_ = direct_3d()->getTextureManager()->load( "title-bg", "media/image/title-bg.png" );
}

TitleScene::~TitleScene()
{
	// direct_3d()->getTextureManager()->unload( "title-bg" );
}

/**
 * ƒƒCƒ“ƒ‹[ƒvˆ—
 *
 */
void TitleScene::update()
{
	if ( input()->push( Input::A ) )
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
			fade_alpha_ = 1.f;
		}
	}

	if ( title_bg_scale_ == -1.f )
	{
		sequence_ = SEQUENCE_TITLE_FIX;
	}
	else if ( bgm_->get_current_position() >= 19.f )
	{
		if ( sequence_ < SEQUENCE_TITLE_BG )
		{
			sequence_ = SEQUENCE_TITLE_BG;
			fade_alpha_ = 1.f;
		}
	}
	else if ( bgm_->get_current_position() >= 9.5f )
	{
		if ( sequence_ < SEQUENCE_TITLE_LOGO )
		{
			sequence_ = SEQUENCE_TITLE_LOGO;
			fade_alpha_ = 1.f;
		}
	}

	if ( sequence_ >= SEQUENCE_TITLE_FIX )
	{
		title_bg_scale_ = -1.f + cos( title_bg_scale_cycle_ ) * 0.05f - 0.05f;
		title_bg_scale_cycle_ += 0.01f;
	}
	else if ( sequence_ >= SEQUENCE_TITLE_BG )
	{
		if ( title_bg_scale_ > 0.f )
		{
			title_bg_scale_ = math::chase( title_bg_scale_, -1.f, 0.002f );
		}
		else
		{
			title_bg_scale_ = math::chase( title_bg_scale_, -1.f, 0.004f );
		}
	}

	fade_alpha_ = math::chase( fade_alpha_, 0.f, 0.01f );
	
}

/**
 * •`‰æ
 */
bool TitleScene::render()
{
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->BeginScene() );

	D3DXHANDLE technique = direct_3d()->getEffect()->GetTechniqueByName( "technique_0" );
	direct_3d()->getEffect()->SetTechnique( technique );

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

	return true;
}

} // namespace blue_sky