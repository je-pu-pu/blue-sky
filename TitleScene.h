#ifndef BLUE_SKY_TITLE_SCENE_H
#define BLUE_SKY_TITLE_SCENE_H

#include "Scene.h"

namespace blue_sky
{

/**
 * タイトル画面の処理を管理する
 *
 */
class TitleScene : public Scene
{
public:
	TitleScene( const GameMain* );
	~TitleScene();

	void update();				///< メインループ
	void render();				///< 描画

}; // class Scene

} // namespace GamePlayScene

#endif // BLUE_SKY_SCENE_H