#ifndef BLUE_SKY_GRAPHICS_MANAGER_H
#define BLUE_SKY_GRAPHICS_MANAGER_H

#include "type.h"
#include <game/GraphicsManager.h>

namespace blue_sky
{

class DrawingMesh;
class DrawingLine;

/**
 * グラフィック管理クラス
 *
 */
class GraphicsManager : public game::GraphicsManager
{
public:

public:
	GraphicsManager() { }
	virtual ~GraphicsManager() { }

	virtual DrawingMesh* create_drawing_mesh() = 0;
	virtual DrawingLine* create_drawing_line() = 0;

	DrawingMesh* load_drawing_mesh( const char_t* );
	DrawingLine* load_drawing_line( const char_t* );

}; // class GraphicsManager

} // namespace blue_sky

#endif // BLUE_SKY_GRAPHICS_MANAGER_H
