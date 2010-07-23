#ifndef BLUE_SKY_STAGE_SELECT_SCENE_H
#define BLUE_SKY_STAGE_SELECT_SCENE_H

#include "Scene.h"
#include <win/Rect.h>
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
	};

	typedef std::list< Stage* > StageList;

private:
	int page_;
	StageList stage_list_;

	Texture* sprite_texture_;

	Sound* ok_;

	win::Rect cursor_src_rect_;
	win::Rect left_allow_src_rect_;
	win::Rect right_allow_src_rect_;
	win::Rect stage_src_rect_;

	int get_margin() const { return 5; }

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