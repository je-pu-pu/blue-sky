#ifndef BLUE_SKY_GRAPHICS_MANAGER_H
#define BLUE_SKY_GRAPHICS_MANAGER_H

#include "type.h"
#include <game/GraphicsManager.h>
#include <common/safe_ptr.h>
#include <common/auto_ptr.h>

class SkinningAnimationSet;
class FbxFileLoader;

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

private:
	common::auto_ptr< FbxFileLoader> fbx_file_loader_;

public:
	GraphicsManager();
	virtual ~GraphicsManager();

	virtual DrawingMesh* create_drawing_mesh() = 0;
	virtual DrawingLine* create_drawing_line() = 0;

	void setup_loader();
	void cleanup_loader();

	DrawingMesh* load_drawing_mesh( const char_t*, common::safe_ptr< SkinningAnimationSet >& );
	DrawingLine* load_drawing_line( const char_t* );

}; // class GraphicsManager

} // namespace blue_sky

#endif // BLUE_SKY_GRAPHICS_MANAGER_H
