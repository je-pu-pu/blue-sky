#ifndef BLUE_SKY_STAGE_SELECT_SCENE_H
#define BLUE_SKY_STAGE_SELECT_SCENE_H

#include "Scene.h"
#include <win/Rect.h>
#include <vector>
#include <list>

namespace blue_sky
{

/**
 * ステージ選択画面の処理を管理する
 *
 */
class StageSelectScene : public Scene
{
public:
	class Stage
	{
	public:
		std::string name;
		Texture* texture;
		win::Rect rect;
		bool cleared;
		bool completed;
	};

	typedef std::list< Stage* > StageList;
	typedef std::vector< win::Rect > RectList;

	static std::string get_stage_dir_name_by_page( int );
	static std::string get_stage_prefix_by_page( int );
	static int get_max_story_page() { return 3; }

	void check_story_completed();
	bool is_final_stage_open() const;

private:
	int page_;
	int stage_count_;
	bool is_story_completed_;

	StageList stage_list_;

	Texture* sprite_texture_;
	Texture* bg_texture_;

	Sound* ok_;
	Sound* click_;

	win::Rect cursor_src_rect_;
	win::Rect left_allow_src_rect_;
	win::Rect right_allow_src_rect_;
	win::Rect stage_src_rect_;

	RectList circle_src_rect_list_;
	RectList face_src_rect_list_;

	int get_max_stage_per_page() const { return 4; }
	int get_margin() const { return 5; }

	void update_page( int );

	void clear_stage_list();
	void update_stage_list();

	bool has_prev_page() const;
	bool has_next_page() const;

	bool is_mouse_on_left_allow() const;
	bool is_mouse_on_right_allow() const;

	Stage* get_pointed_stage() const;
	
	win::Rect get_stage_dst_rect( const Stage*, int ) const;

public:
	StageSelectScene( const GameMain* );
	~StageSelectScene();

	void update();				///< メインループ
	void render();				///< 描画

}; // class StageSelectScene

} // namespace blue_sky

#endif // BLUE_SKY_STAGE_SELECT_SCENE_H