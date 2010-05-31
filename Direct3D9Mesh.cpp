#include "Direct3D9Mesh.h"
#include "Direct3D9.h"

#include <common/exception.h>

#define FAIL_CHECK( x ) if ( FAILED( x ) ) { COMMON_THROW_EXCEPTION; }

Direct3D9Mesh::Direct3D9Mesh( Direct3D9* direct_3d )
	: direct_3d_( direct_3d )
	, mesh_( 0 )
{

}

Direct3D9Mesh::~Direct3D9Mesh()
{
	if ( mesh_ )
	{
		mesh_->Release();
	}
}

bool Direct3D9Mesh::loadX( const char* file_name )
{
	LPD3DXBUFFER adjacency_buffer;
	LPD3DXBUFFER materials_buffer;
	DWORD material_count;

	FAIL_CHECK( D3DXLoadMeshFromX( file_name, D3DXMESH_SYSTEMMEM, direct_3d_->getDevice(), & adjacency_buffer, & materials_buffer, 0, & material_count, & mesh_ ) );
}