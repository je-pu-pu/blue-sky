#ifndef BLUE_SKY_STORY_TEXT_SCENE_H
#define BLUE_SKY_STORY_TEXT_SCENE_H

#include "Scene.h"
#include <common/chase_value.h>

namespace blue_sky
{

/**
 * ストーリー画面の処理を管理する
 *
 */
class StoryTextScene : public Scene
{
public:

private:
	std::string		text_;
	float			text_y_;
	float			text_y_target_;
	float			text_scroll_speed_;
	D3DCOLOR		text_color_;
	
	std::string		next_scene_name_;

	Texture*		sprite_texture_;
	Texture*		bg_texture_;
	int				bg_width_;
	int				bg_height_;

	common::chase_value< float > bg_scale_;

	void load_story_text_file( const char* );

public:
	StoryTextScene( const GameMain*, const char*, const char* );
	~StoryTextScene();

	void update();				///< メインループ
	bool render();				///< 描画

}; // class StoryTextScene

} // namespace blue_sky

#endif // BLUE_SKY_STORY_TEXT_SCENE_H