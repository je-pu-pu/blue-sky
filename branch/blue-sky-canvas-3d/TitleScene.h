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
	enum Sequence
	{
		SEQUENCE_LOGO = 0,
		SEQUENCE_TITLE_LOGO,
		SEQUENCE_TITLE_FIX
	};

private:
	Texture* title_texture_;
	Texture* title_bg_texture_;
	Texture* cloth_texture_;

	DrawingModel* brand_logo_model_;
	DrawingModel* title_logo_model_;

	Sound* ok_;
	Sound* bgm_;

	int_t sequence_;
	float_t sequence_elapsed_time_;

public:
	TitleScene( const GameMain* );
	~TitleScene();

	void update();				///< メインループ
	void render();				///< 描画

}; // class TitleScene

} // namespace blue_sky

#endif // BLUE_SKY_TITLE_SCENE_H