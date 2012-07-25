#include "DrawingModel.h"
#include "DrawingLine.h"
#include "Direct3D11Mesh.h"

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
	GameMain::Direct3D* direct_3d = GameMain::get_instance()->get_direct_3d();

	/// @todo Direct3D ‚ð“n‚³‚È‚¢
	mesh_ = new DrawingMesh( direct_3d );
	line_ = new DrawingLine( direct_3d );

	mesh_->load_obj( ( string_t( "media/model/" ) + name + ".obj" ).c_str() );
	line_->load_obj( ( string_t( "media/model/" ) + name + "-line.obj" ).c_str() );
}

} // namespace blue_sky