#ifndef GAME_GRAPHICS_MANAGER_H
#define GAME_GRAPHICS_MANAGER_H

#include <common/safe_ptr.h>
#include <type/type.h>

namespace game
{

/**
 * 描画されるオブジェクトの基底クラス
 *
 */
class RenderingObject
{
public:
	RenderingObject() { }
	virtual ~RenderingObject() { }

	virtual void render() const = 0;
};

class Mesh;

/**
 * 線
 *
 */
class Line : public RenderingObject { };

/**
 * グラフィック管理クラス
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
