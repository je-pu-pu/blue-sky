#pragma once

#include "Scene.h"
#include <win/Rect.h>
#include <type/type.h>
#include <vector>
#include <list>

namespace blue_sky
{

/**
 * �X�e�[�W�I����ʂ̏������Ǘ�����
 *
 */
class StageSelectScene : public Scene
{
public:

	class Stage
	{
	public:
		string_t name;
		Texture* texture;
		win::Rect rect;
		bool cleared;
		bool completed;
	};

	using StageList	= std::list< Stage* >;
	using RectList	= std::vector< win::Rect >;

	static string_t get_stage_dir_name_by_page( int );
	static string_t get_stage_prefix_by_page( int );
	static int get_max_story_page() { return 3; }

	void check_story_completed();
	bool is_final_stage_open() const;

	bool is_story_page() const { return page_ < get_max_story_page(); }

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
	win::Rect left_arrow_src_rect_;
	win::Rect right_arrow_src_rect_;
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

	bool is_mouse_on_left_arrow() const;
	bool is_mouse_on_right_arrow() const;

	Stage* get_pointed_stage() const;
	
	win::Rect get_stage_dst_rect( const Stage*, int ) const;

public:
	explicit StageSelectScene( const GameMain* );
	~StageSelectScene();

	void update() override;				///< ���C�����[�v
	void render() override;				///< �`��

}; // class StageSelectScene

} // namespace blue_sky
