#ifndef BLUE_SKY_TITLE_SCENE_H
#define BLUE_SKY_TITLE_SCENE_H

#include "Scene.h"

namespace blue_sky
{

/**
 * �^�C�g����ʂ̏������Ǘ�����
 *
 */
class TitleScene : public Scene
{
public:
	TitleScene( const GameMain* );
	~TitleScene();

	void update();				///< ���C�����[�v
	void render();				///< �`��

}; // class Scene

} // namespace GamePlayScene

#endif // BLUE_SKY_SCENE_H