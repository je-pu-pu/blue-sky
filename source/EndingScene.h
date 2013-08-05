#ifndef BLUE_SKY_ENDING_SCENE_H
#define BLUE_SKY_ENDING_SCENE_H

#include "Scene.h"
#include <common/auto_ptr.h>
#include <vector>

class DrawingModel;

namespace game
{

class ElapsedTimer;
class Texture;

}

namespace blue_sky
{

/**
 * エンディング画面の処理を管理する
 *
 */
class EndingScene : public Scene
{
public:
	typedef std::vector< DrawingModel* > DrawingModelList;

private:
	common::auto_ptr< game::ElapsedTimer > elapsed_timer_;

	DrawingModelList drawing_model_list_;
	uint_t current_drawing_model_index_;
	DrawingModel* current_drawing_model_;
	float_t drawing_model_elapsed_time_;
	float_t drawing_model_stop_time_;
	float_t drawing_model_offset_;

	Texture* bg_texture_;
	Sound* bgm_;

	void load_sequence_file();

	void render_bg();
	void render_drawing_line();

	int get_visible_drawing_line_part_count() const;

	Sound* get_bgm() override { return bgm_; }

public:
	EndingScene( const GameMain* );
	~EndingScene();

	void update() override;				///< メインループ
	void render() override;				///< 描画

}; // class EndingScene

} // namespace blue_sky

#endif // BLUE_SKY_ENDING_SCENE_H