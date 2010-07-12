#include "TitleScene.h"

#include "Input.h"
#include "SoundManager.h"
#include "Sound.h"

#include "Direct3D9.h"
#include "Direct3D9Mesh.h"
#include "Direct3D9TextureManager.h"
#include "DirectX.h"

#include <common/math.h>
#include <common/exception.h>

namespace blue_sky
{

Direct3D9Mesh* mesh_;

TitleScene::TitleScene( const GameMain* game_main )
	: Scene( game_main )
{
	mesh_ = new Direct3D9Mesh( direct_3d() );
	mesh_->load_x( "media/model/building-a.x" );

	sound_manager()->load_music( "bgm", "title" )->play( false );
	
	direct_3d()->getTextureManager()->load( "title", "media/image/title.png" );
	direct_3d()->getTextureManager()->load( "title-bg", "media/image/title-bg.png" );
}

TitleScene::~TitleScene()
{
	delete mesh_;
}

/**
 * ƒƒCƒ“ƒ‹[ƒvˆ—
 *
 */
void TitleScene::update()
{
	if ( input()->push( Input::A ) )
	{
		set_next_scene( "stage_select" );
	}
}

/**
 * •`‰æ
 */
void TitleScene::render()
{
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->BeginScene() );

//	D3DXHANDLE technique = direct_3d()->getEffect()->GetTechniqueByName( "technique_0" );
//	direct_3d()->getEffect()->SetTechnique( technique );

	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloat( "brightness", 1.f ) );

	UINT pass_count = 0;

	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0xFF, 0xAA, 0x11 ), 1.f, 0 ) );

//	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->Begin( & pass_count, 0 ) );
//	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->BeginPass( 0 ) );

	DIRECT_X_FAIL_CHECK( direct_3d()->getSprite()->Begin( D3DXSPRITE_ALPHABLEND ) );

	D3DXMATRIXA16 transform;
	D3DXMATRIXA16 s, t;

	if ( input()->press( Input::UP ) )
	{
		static float scale = 2.f;
		scale = math::chase( scale, -1.f, 0.002f );

		D3DXMATRIXA16 ds;

		RECT src_rect = { 0, 0, 2048, 1024 };
		D3DXVECTOR3 center( ( src_rect.right - src_rect.left ) * 0.5f, ( src_rect.bottom - src_rect.top ) * 0.5f, 0.f );
		D3DXVECTOR3 position( get_width() * 0.5f, get_height() * 0.5f, 0.f );

		D3DXMatrixTranslation( & t, get_width() * 0.5f, get_height() * 0.5f, 0.f );

		float ratio = static_cast< float >( get_height() ) / static_cast< float >( src_rect.bottom - src_rect.top );
		D3DXMatrixScaling( & s, ratio * scale, ratio * scale, 1.f );

		// D3DXMatrixRotationZ( & r, scale * 4.f );

		transform = s * t;
		direct_3d()->getSprite()->SetTransform( & transform );
		direct_3d()->getSprite()->Draw(  direct_3d()->getTextureManager()->get( "title-bg" ), & src_rect, & center, 0, 0xFFFFFFFF );
	}
	if ( ! input()->press( Input::B ) )
	{
		RECT src_rect = { 0, 0, 400, 128 };
		D3DXVECTOR3 center( ( src_rect.right - src_rect.left ) * 0.5f, ( src_rect.bottom - src_rect.top ) * 0.5f, 0.f );
		
		D3DXMatrixTranslation( & t, get_width() * 0.5f, get_height() * 0.5f, 0.f );

		transform = t;
		direct_3d()->getSprite()->SetTransform( & transform );
		direct_3d()->getSprite()->Draw( direct_3d()->getTextureManager()->get( "title" ), & src_rect, & center, 0, 0xFFFFFFFF );
	}
	if ( input()->press( Input::B ) )
	{
		RECT src_rect = { 0, 128, 540, 448 };
		D3DXVECTOR3 center( ( src_rect.right - src_rect.left ) * 0.5f, ( src_rect.bottom - src_rect.top ) * 0.5f, 0.f );
		D3DXVECTOR3 position( get_width() * 0.5f, get_height() * 0.5f, 0.f );

		D3DXMatrixIdentity( & transform );
		direct_3d()->getSprite()->SetTransform( & transform );
		direct_3d()->getSprite()->Draw( direct_3d()->getTextureManager()->get( "title" ), & src_rect, & center, & position, 0xFFFFFFFF );
	}

	DIRECT_X_FAIL_CHECK( direct_3d()->getSprite()->End() );

//	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->EndPass() );
//	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->End() );

	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->EndScene() );
}

} // namespace blue_sky