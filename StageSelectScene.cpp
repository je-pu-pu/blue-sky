#include "StageSelectScene.h"
#include "Input.h"
#include "SoundManager.h"

#include "Direct3D9.h"
#include "Direct3D9Mesh.h"
#include "Direct3D9TextureManager.h"
#include "DirectX.h"

#include <game/Sound.h>

#include <windows/Rect.h>

#include <common/exception.h>

namespace blue_sky
{

StageSelectScene::StageSelectScene( const GameMain* game_main )
	: Scene( game_main )
	, sprite_texture_( 0 )
	, ok_( 0 )
{
	sprite_texture_ = direct_3d()->getTextureManager()->load( "sprite", "media/image/title.png" );

	ok_ = sound_manager()->get_sound( "ok" );
}

StageSelectScene::~StageSelectScene()
{
	
}

/**
 * ƒƒCƒ“ƒ‹[ƒvˆ—
 *
 */
void StageSelectScene::update()
{
	if ( input()->push( Input::A ) )
	{
		ok_->play( false );
		set_next_scene( "game_play" );
	}
}

/**
 * •`‰æ
 */
void StageSelectScene::render()
{
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->BeginScene() );
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0x11, 0xAA, 0xFF ), 1.f, 0 ) );
	DIRECT_X_FAIL_CHECK( direct_3d()->getSprite()->Begin( D3DXSPRITE_ALPHABLEND ) );

	D3DXMATRIXA16 transform;

	// Allow
	const float allow_margin = 5.f;

	{
		win::Rect src_rect = win::Rect::Size( 128, 704, 82, 126 );
		D3DXVECTOR3 center( src_rect.width() * 0.5f, src_rect.height() * 0.5f, 0.f );

		D3DXMatrixTranslation( & transform, 0.f + src_rect.width() * 0.5f + allow_margin, get_height() * 0.5f, 0.f );

		direct_3d()->getSprite()->SetTransform( & transform );
		direct_3d()->getSprite()->Draw( sprite_texture_, & src_rect.get_rect(), & center, 0, 0xFFFFFFFF );
	}
	{
		win::Rect src_rect = win::Rect::Size( 256, 704, 86, 126 );
		D3DXVECTOR3 center( src_rect.width() * 0.5f, src_rect.height() * 0.5f, 0.f );

		D3DXMatrixTranslation( & transform, get_width() - src_rect.width() * 0.5f - allow_margin, get_height() * 0.5f, 0.f );

		direct_3d()->getSprite()->SetTransform( & transform );
		direct_3d()->getSprite()->Draw( sprite_texture_, & src_rect.get_rect(), & center, 0, 0xFFFFFFFF );
	}

	// Cursor
	{
		win::Rect src_rect( 0, 702, 91, 841 );

		D3DXVECTOR3 center( src_rect.width() * 0.5f, src_rect.height() * 0.5f, 0.f );
		
		D3DXMatrixTranslation( & transform, static_cast< float >( input()->get_mouse_x() ) + src_rect.width() * 0.5f - 5.f, static_cast< float >( input()->get_mouse_y() ) + src_rect.height() * 0.5f - 5.f, 0.f );

		direct_3d()->getSprite()->SetTransform( & transform );
		direct_3d()->getSprite()->Draw( sprite_texture_, & src_rect.get_rect(), & center, 0, 0xFFFFFFFF );
	}

	DIRECT_X_FAIL_CHECK( direct_3d()->getSprite()->End() );
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->EndScene() );
}

} // namespace blue_sky