#ifndef BLUE_SKY_CANVAS_TEST_SCENE_H
#define BLUE_SKY_CANVAS_TEST_SCENE_H

#include "Scene.h"
#include <win/Tablet.h>
#include <common/safe_ptr.h>
#include <memory>

namespace game
{
	class Mesh;
}

namespace win
{
	class Tablet;
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
	typedef win::Tablet Tablet;

private:
	common::safe_ptr< Tablet > tablet_;

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
