#include "EndingScene.h"
#include "DirectShow.h"
#include "Direct3D9.h"
#include "App.h"
#include "SoundManager.h"

namespace blue_sky
{

EndingScene::EndingScene( const GameMain* game_main )
	: Scene( game_main )
	, direct_show_( 0 )
{
	direct_3d()->set_depth_stencil( false );
	direct_3d()->set_multi_sample( 0, 0 );
	direct_3d()->reset( true );

	sound_manager()->stop_all();
	sound_manager()->unload_all();

	direct_show_ = new DirectShow( App::GetInstance()->GetWindowHandle() );
}

EndingScene::~EndingScene()
{
	direct_3d()->set_depth_stencil( true );
	direct_3d()->reset( true );

	delete direct_show_;
}

/**
 * ƒƒCƒ“ƒ‹[ƒvˆ—
 *
 */
void EndingScene::update()
{
	
}

/**
 * •`‰æ
 */
bool EndingScene::render()
{ 
	return false;
}

} // namespace blue_sky