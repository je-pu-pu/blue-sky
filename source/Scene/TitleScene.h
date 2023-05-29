#pragma once

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
	static inline const char_t* name = "title";

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

	Model* brand_logo_model_;
	Model* title_logo_model_;

	Sound* ok_;
	Sound* bgm_;

	int_t sequence_;
	float_t sequence_elapsed_time_;

	Sound* get_bgm() override { return bgm_; }

public:
	explicit TitleScene();
	~TitleScene();

	void update() override;				///< メインループ
	void render() override;				///< 描画

}; // class TitleScene

} // namespace blue_sky
