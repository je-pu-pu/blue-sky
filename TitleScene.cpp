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
 * メインループ処理
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
 * 描画
 */
void TitleScene::render()
{
	
}

} // namespace blue_sky