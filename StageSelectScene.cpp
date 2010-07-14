#include "StageSelectScene.h"
#include "Input.h"
#include "SoundManager.h"

#include "Direct3D9.h"
#include "Direct3D9Mesh.h"
#include "DirectX.h"

#include <game/Sound.h>

#include <common/exception.h>

namespace blue_sky
{

StageSelectScene::StageSelectScene( const GameMain* game_main )
	: Scene( game_main )
	, ok_( 0 )
{
	ok_ = sound_manager()->load( "ok" );
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

	D3DXHANDLE technique = direct_3d()->getEffect()->GetTechniqueByName( "technique_0" );
	direct_3d()->getEffect()->SetTechnique( technique );

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
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0x00, 0x00, 0x66 ), 1.f, 0 ) );

	D3DXMATRIXA16 world;
	D3DXMATRIXA16 ortho;
	D3DXMATRIXA16 WorldViewProjection;

	D3DXMatrixOrthoLH( & ortho, 10, 10, 0.f, 1.f );

	WorldViewProjection = ortho;
	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->SetMatrix( "WorldViewProjection", & WorldViewProjection ) );
	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->CommitChanges() );

	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->EndPass() );
	DIRECT_X_FAIL_CHECK( direct_3d()->getEffect()->End() );

	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->EndScene() );
}

} // namespace blue_sky