#pragma once

#include "Scene.h"
#include <win/Rect.h>
#include <type/type.h>
#include <memory>
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
	static inline const char_t* name = "stage_select";

	class Stage
	{
	public:
		string_t name;
		TextureHandle texture;		///< ステージの絵（ハンドルで持ち、描くたびに引く）
		win::Rect rect;
		bool cleared = false;
		bool completed = false;
	};

	using StageList	= std::list< std::unique_ptr< Stage > >;
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

	// リソースはハンドルで持ち、使うたびに引く（以前は生ポインタで、解放・差し替えの後に古いものを指し得た）
	TextureHandle sprite_texture_;
	TextureHandle bg_texture_;

	SoundHandle ok_;
	SoundHandle click_;

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
	explicit StageSelectScene( GameMain& game_main );
	~StageSelectScene();

	void update() override;				///< メインループ
	void render() override;				///< 描画

}; // class StageSelectScene

} // namespace blue_sky
