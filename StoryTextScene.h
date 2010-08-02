#ifndef BLUE_SKY_STORY_TEXT_SCENE_H
#define BLUE_SKY_STORY_TEXT_SCENE_H

#include "Scene.h"

namespace blue_sky
{

/**
 * �X�g�[���[��ʂ̏������Ǘ�����
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
	
	std::string		next_scene_name_;

	Texture*		bg_texture_;

	void load_story_text_file( const char* );

public:
	StoryTextScene( const GameMain*, const char*, const char* );
	~StoryTextScene();

	void update();				///< ���C�����[�v
	bool render();				///< �`��

}; // class StoryTextScene

} // namespace blue_sky

#endif // BLUE_SKY_STORY_TEXT_SCENE_H