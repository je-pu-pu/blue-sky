#include "DrawingModel.h"
#include "DrawingMesh.h"
#include "DrawingLine.h"

#include "FbxFileLoader.h"

#include "GraphicsManager.h"

#include "GameMain.h"

namespace blue_sky
{

DrawingModel::DrawingModel()
{

}

DrawingModel::~DrawingModel()
{

}

void DrawingModel::load( const char_t* name )
{
	mesh_ = GameMain::get_instance()->get_graphics_manager()->load_drawing_mesh( name );
	line_ = GameMain::get_instance()->get_graphics_manager()->load_drawing_line( name );
}

} // namespace blue_sky