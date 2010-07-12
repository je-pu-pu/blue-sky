#include "TitleScene.h"

#include "Input.h"
#include "SoundManager.h"
#include "Sound.h"

#include "Direct3D9.h"
#include "Direct3D9Mesh.h"
#include "Direct3D9TextureManager.h"
#include "DirectX.h"

#include <common/exception.h>

namespace blue_sky
{

Direct3D9Mesh* mesh_;

TitleScene::TitleScene( const GameMain* game_main )
	: Scene( game_main )
{
	mesh_ = new Direct3D9Mesh( direct_3d() );
	mesh_->load_x( "media/model/building-a.x" );

	sound_manager()->load_music( "bgm", "title" )->play( true );
	
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

	D3DXHANDLE technique = direct_3d()->getEffect()->GetTechniqueByName( "technique_0" );
	direct_3d()->getEffect()->SetTechnique( technique );

	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetFloat( "brightness", 1.f ) );

	UINT pass_count = 0;

	direct_3d()->getEffect()->Begin( & pass_count, 0 );
	direct_3d()->getEffect()->BeginPass( 0 );

	D3DVIEWPORT9 view_port;
	view_port.X = 0;
	view_port.Y = 0;
	view_port.Width	= get_width();
	view_port.Height = get_height();
	view_port.MinZ = 0.f;
	view_port.MaxZ = 1.f;

	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->SetViewport( & view_port ) );
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0xFF, 0xAA, 0x11 ), 1.f, 0 ) );

	DIRECT_X_FAIL_CHECK( direct_3d()->getSprite()->Begin( 0 ) );

	RECT src_rect = { 0, 0, 1024, 1707 };
	D3DXVECTOR3 center( src_rect.right * 0.5f, src_rect.bottom / 2.f, 0 );
	D3DXVECTOR3 position( get_width() * 0.5f, get_height() * 0.5f, 0.f );
	
	static float scale = 1.f;
	static float oy = 0.f;
	scale -= 0.0001f;
	oy -= 0.1f;

	D3DXMATRIXA16 transform;

	{
		D3DXMATRIXA16 s, t;

		D3DXMatrixScaling( & s, scale, scale, scale );
		D3DXMatrixTranslation( & t, 0.f, oy, 0.f );

		transform = s * t;

		direct_3d()->getSprite()->SetTransform( & transform );
		direct_3d()->getSprite()->Draw( direct_3d()->getTextureManager()->get( "title-bg" ), & src_rect, & center, & position, D3DCOLOR_XRGB( 255, 255, 255 ) );
	}
	if ( ! input()->press( Input::B ) )
	{
		RECT src_rect = { 0, 0, 389, 117 };
		D3DXVECTOR3 center( 389 / 2, 117 / 2, 0.f );
		D3DXVECTOR3 position( get_width() * 0.5f, get_height() * 0.5f, 0.f );

		D3DXMatrixIdentity( & transform );
		direct_3d()->getSprite()->SetTransform( & transform );
		direct_3d()->getSprite()->Draw( direct_3d()->getTextureManager()->get( "title" ), & src_rect, & center, & position, D3DCOLOR_XRGB( 255, 255, 255 ) );
	}
	if ( input()->press( Input::B ) ) {
		RECT src_rect = { 0, 128, 528, 128 + 309 };
		D3DXVECTOR3 center( 528 / 2, 309 / 2, 0.f );
		D3DXVECTOR3 position( get_width() * 0.5f, get_height() * 0.5f, 0.f );

		D3DXMatrixIdentity( & transform );
		direct_3d()->getSprite()->SetTransform( & transform );
		direct_3d()->getSprite()->Draw( direct_3d()->getTextureManager()->get( "title" ), & src_rect, & center, & position, D3DCOLOR_XRGB( 255, 255, 255 ) );
	}

	DIRECT_X_FAIL_CHECK( direct_3d()->getSprite()->End() );

	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->EndPass() );
	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->End() );

	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->EndScene() );
}

} // namespace blue_sky