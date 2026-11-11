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
	// リソースはハンドルで持ち、使うたびに引く（以前は生ポインタで、解放・差し替えの後に古いものを指し得た）
	TextureHandle title_bg_texture_;
	TextureHandle cloth_texture_;

	ModelHandle brand_logo_model_;
	ModelHandle title_logo_model_;

	SoundHandle ok_;
	SoundHandle bgm_;

	int_t sequence_;
	float_t sequence_elapsed_time_;

	Sound* get_bgm() override { return resolve( bgm_, "bgm" ); }

public:
	explicit TitleScene( GameMain& game_main );
	~TitleScene();

	void update() override;				///< メインループ
	void render() override;				///< 描画

	EscapeBehavior get_escape_behavior() const override { return EscapeBehavior::QUIT; }

}; // class TitleScene

} // namespace blue_sky
