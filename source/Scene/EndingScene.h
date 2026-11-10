#pragma once

#include "Scene.h"
#include <vector>
#include <memory>

namespace game
{

class ElapsedTimer;
class Texture;

}

namespace blue_sky
{

	namespace graphics
	{
		class Model;
		class Line;
	}

/**
 * エンディング画面の処理を管理する
 *
 */
class EndingScene : public Scene
{
public:
	static inline const char_t* name = "ending";

	using Model		= graphics::Model;
	using Line		= graphics::Line;
	using ModelList	= std::vector< ModelHandle >;

private:
	std::unique_ptr< game::ElapsedTimer > elapsed_timer_;

	// リソースはハンドルで持ち、使うたびに引く（以前は生ポインタで、解放・差し替えの後に古いものを指し得た）
	ModelList drawing_model_list_;
	uint_t current_drawing_model_index_;
	float_t drawing_model_elapsed_time_;
	float_t drawing_model_stop_time_;
	float_t drawing_model_offset_;

	int line_type_;
	float_t drawing_accent_scale_;

	TextureHandle bg_texture_;
	SoundHandle bgm_;
	SoundHandle switch_sound_;
	SoundHandle click_sound_;

	bool in_fade_out_;

	void load_sequence_file();

	void render_bg();
	void render_drawing_line();

	int get_visible_drawing_line_part_count() const;

	/// 今描いているモデル（引けなければ nullptr）
	const Model* get_current_drawing_model() const { return resolve( drawing_model_list_[ current_drawing_model_index_ ], "drawing model" ); }

	Sound* get_bgm() override { return resolve( bgm_, "opening-of-the-day" ); }

public:
	explicit EndingScene( GameMain& game_main );
	~EndingScene();

	void update() override;				///< メインループ
	void render() override;				///< 描画

}; // class EndingScene

} // namespace blue_sky
