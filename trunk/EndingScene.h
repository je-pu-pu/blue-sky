#ifndef BLUE_SKY_ENDING_SCENE_H
#define BLUE_SKY_ENDING_SCENE_H

#include "Scene.h"

class DirectShow;

namespace blue_sky
{

/**
 * エンディング画面の処理を管理する
 *
 */
class EndingScene : public Scene
{
public:

private:
	DirectShow* direct_show_;


public:
	EndingScene( const GameMain* );
	~EndingScene();

	void update();				///< メインループ
	bool render();				///< 描画

}; // class EndingScene

} // namespace blue_sky

#endif // BLUE_SKY_ENDING_SCENE_H