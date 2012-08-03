#include "EndingScene.h"
#include "Direct3D11.h"
#include "DirectWrite.h"
#include "DirectX.h"
#include "App.h"
#include "SoundManager.h"
#include "Sound.h"

namespace blue_sky
{

EndingScene::EndingScene( const GameMain* game_main )
	: Scene( game_main )
	, direct_show_( 0 )
{
	get_direct_3d()->set_depth_stencil( false );
	get_direct_3d()->set_multi_sample( 0, 0 );
	get_direct_3d()->reset( true );

	get_sound_manager()->stop_all();
	get_sound_manager()->unload_all();

	get_sound_manager()->load_music( "ending" )->play( false );
}

EndingScene::~EndingScene()
{
	get_direct_3d()->set_depth_stencil( true );
	get_direct_3d()->reset( true );

	delete direct_show_;
}

/**
 * メインループ処理
 *
 */
void EndingScene::update()
{
	
}

/**
 * 描画
 */
bool EndingScene::render()
{ 
	static int y = get_height();

	y--;

	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->BeginScene() );
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB( 0x00, 0x00, 0x00 ), 1.f, 0 ) );

	direct_3d()->getFont()->draw_text( 0, y, "安藤です。\nどうもです。", D3DCOLOR_ARGB( 127, 255, 255, 255 ) );

	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->EndScene() );

	return true;

	// return false;
}

} // namespace blue_sky