#pragma once

#include "Scene.h"
#include <common/chase_value.h>
#include <list>

namespace core
{
	class BgSpriteLayer;
}

namespace blue_sky
{

/**
 * ストーリー画面の処理を管理する
 *
 */
class StoryTextScene : public Scene
{
public:
	using BgSpriteLayer		= core::BgSpriteLayer;
	using BgSpriteLayerList	= std::list< BgSpriteLayer* >;

private:
	std::wstring	text_;
	float			text_y_;
	float			text_y_target_;
	float			text_scroll_speed_;
	Color			text_color_;
	Color			text_border_color_;
	
	std::string		next_scene_name_;
	std::string		next_stage_name_;

	Texture*		sprite_texture_;
	
	BgSpriteLayerList	bg_sprite_layer_list_;

	Sound*			bgm_;
	Sound*			sound_;

	bool			is_skipped_;

	void load_story_text_file( const char* );

public:
	StoryTextScene( const char*, const char* );
	~StoryTextScene();

	void update() override;				///< メインループ
	void render() override;				///< 描画

}; // class StoryTextScene

} // namespace blue_sky
