#include "DrawingMesh.h"

namespace blue_sky
{

DrawingMesh::DrawingMesh( Mesh* mesh )
	: mesh_( mesh )
{

}

bool DrawingMesh::load_obj( const char_t* file_name )
{
	return mesh_->load_obj( file_name );
}

bool DrawingMesh::load_fbx( const char_t* file_name )
{
	return mesh_->load_fbx( file_name );
}

void DrawingMesh::render() const
{
	mesh_->render();
}

} // namespace blue_sky
