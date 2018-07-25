#pragma once

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

	namespace graphics
	{
		class Model;
		class Line;
	}

/**
 * �G���f�B���O��ʂ̏������Ǘ�����
 *
 */
class EndingScene : public Scene
{
public:
	typedef blue_sky::graphics::Model Model;
	typedef blue_sky::graphics::Line Line;
	typedef std::vector< Model* > DrawingModelList;

private:
	common::auto_ptr< game::ElapsedTimer > elapsed_timer_;

	DrawingModelList drawing_model_list_;
	uint_t current_drawing_model_index_;
	Model* current_drawing_model_;
	float_t drawing_model_elapsed_time_;
	float_t drawing_model_stop_time_;
	float_t drawing_model_offset_;

	int line_type_;
	float_t drawing_accent_scale_;

	Texture* bg_texture_;
	Sound* bgm_;
	Sound* switch_sound_;
	Sound* click_sound_;

	bool in_fade_out_;

	void load_sequence_file();

	void render_bg();
	void render_drawing_line();

	int get_visible_drawing_line_part_count() const;

	Sound* get_bgm() override { return bgm_; }

public:
	explicit EndingScene( const GameMain* );
	~EndingScene();

	void update() override;				///< ���C�����[�v
	void render() override;				///< �`��

}; // class EndingScene

} // namespace blue_sky
