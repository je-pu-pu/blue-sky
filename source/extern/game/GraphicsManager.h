#ifndef GAME_GRAPHICS_MANAGER_H
#define GAME_GRAPHICS_MANAGER_H

#include <common/safe_ptr.h>
#include <type/type.h>

class SkinningAnimationSet;

namespace game
{

/**
 * �`�悳���I�u�W�F�N�g�̊��N���X
 *
 */
class RenderingObject
{
public:
	RenderingObject() { }
	virtual ~RenderingObject() { }

	virtual void render() const = 0;
};

/**
 * ���̕�
 *
 */
class Mesh : public RenderingObject
{
public:
	virtual bool load_obj( const char_t* ) { return false; }
	virtual bool load_fbx( const char_t*, common::safe_ptr< SkinningAnimationSet >& ) { return false; }
};

/**
 * ��
 *
 */
class Line : public RenderingObject { };

/**
 * �O���t�B�b�N�Ǘ��N���X
 *
 */
class GraphicsManager
{
public:
	typedef Mesh Mesh;
	typedef Line Line;

public:
	GraphicsManager() { }
	virtual ~GraphicsManager() { }

	virtual Mesh* create_mesh() = 0;
	virtual Line* create_line() = 0;

}; // class GraphicsManager

} // namespace game

#endif // GAME_GRAPHICS_MANAGER_H
