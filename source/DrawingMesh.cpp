#include "DrawingMesh.h"

namespace blue_sky
{

DrawingMesh::DrawingMesh( Mesh* mesh )
	: mesh_( mesh )
{

}

DrawingMesh::~DrawingMesh()
{
	
}


bool DrawingMesh::load_obj( const char_t* file_name )
{
	return mesh_->load_obj( file_name );
}

bool DrawingMesh::load_fbx( const char_t* file_name, FbxFileLoader* loader, common::safe_ptr< SkinningAnimationSet >& skinning_animation_set )
{
	return mesh_->load_fbx( file_name, loader, skinning_animation_set );
}

DrawingMesh::Material* DrawingMesh::get_material_at( uint_t index )
{
	return mesh_->get_material_at( index );
}

uint_t DrawingMesh::get_material_count() const
{
	return mesh_->get_material_count();
}

void DrawingMesh::bind_to_ia() const
{
	mesh_->bind_to_ia();
}

} // namespace blue_sky
