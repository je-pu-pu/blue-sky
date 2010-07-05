#include "TitleScene.h"
#include "Input.h"

namespace blue_sky
{

TitleScene::TitleScene( const GameMain* game_main )
	: Scene( game_main )
{
	
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
	if ( input()->push( Input::A ) )
	{
		set_next_scene( "game_play" );
	}
}

/**
 * •`‰æ
 */
void TitleScene::render()
{
	
}

} // namespace blue_sky