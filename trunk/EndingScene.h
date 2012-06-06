#ifndef BLUE_SKY_ENDING_SCENE_H
#define BLUE_SKY_ENDING_SCENE_H

#include "Scene.h"

class DirectShow;

namespace blue_sky
{

/**
 * �G���f�B���O��ʂ̏������Ǘ�����
 *
 */
class EndingScene : public Scene
{
public:

private:
	DirectShow* direct_show_;


public:
	EndingScene( const GameMain* );
	~EndingScene();

	void update();				///< ���C�����[�v
	bool render();				///< �`��

}; // class EndingScene

} // namespace blue_sky

#endif // BLUE_SKY_ENDING_SCENE_H