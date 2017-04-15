#ifndef BLUE_SKY_CANVAS_TEST_SCENE_H
#define BLUE_SKY_CANVAS_TEST_SCENE_H

#include "Scene.h"
#include <common/safe_ptr.h>
#include <memory>

namespace game
{

class Mesh;

}

namespace blue_sky
{

/**
 * �`��e�X�g�p�V�[�����Ǘ�����
 *
 */
class CanvasTestScene : public Scene
{
public:
	typedef game::Mesh Mesh;

private:
	std::unique_ptr< Mesh > mesh_;
	common::safe_ptr< Texture > texture_;

protected:

public:
	CanvasTestScene( const GameMain* );
	~CanvasTestScene();

	void update();				///< ���C�����[�v
	void render();				///< �`��

	bool is_clip_cursor_required() { return true; }

}; // class CanvasTestScene

} // namespace blue_sky

#endif // #define BLUE_SKY_CANVAS_TEST_SCENE_H
