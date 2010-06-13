#include "Direct3D9Mesh.h"
#include "Direct3D9.h"

#include <common/exception.h>
#include <common/math.h>

#define FAIL_CHECK( x ) if ( FAILED( x ) ) { COMMON_THROW_EXCEPTION; }

Direct3D9Mesh::Direct3D9Mesh( Direct3D9* direct_3d )
	: direct_3d_( direct_3d )
	, mesh_( 0 )
	, material_count_( 0 )
{

}

Direct3D9Mesh::~Direct3D9Mesh()
{
	if ( mesh_ )
	{
		mesh_->Release();
	}
}

D3DMATERIAL9* materials_ = 0;

bool Direct3D9Mesh::loadX( const char* file_name )
{
	LPD3DXBUFFER adjacency_buffer;
	LPD3DXBUFFER materials_buffer;

	FAIL_CHECK( D3DXLoadMeshFromX( file_name, D3DXMESH_SYSTEMMEM, direct_3d_->getDevice(), & adjacency_buffer, & materials_buffer, 0, & material_count_, & mesh_ ) );

	materials_ = new D3DMATERIAL9[ material_count_ ];

	for ( unsigned int n = 0; n < material_count_; n++ )
	{
		materials_[ n ] = static_cast< D3DXMATERIAL* >( materials_buffer->GetBufferPointer() )[ n ].MatD3D;
		materials_[ n ].Ambient = materials_[ n ].Diffuse;
		/*
		materials_[ n ].Ambient.r = 1.f;
		materials_[ n ].Ambient.g = 0.5f;
		materials_[ n ].Ambient.b = 0.5f;
		*/
	}

	/*
	D3DLIGHT9* light_ = new D3DLIGHT9();
	light_->Position = D3DXVECTOR3( 10.f, 10.f, -10.f );
	light_->Diffuse = D3DXCOLOR( 1.f, 1.f, 1.f, 1.f );

	direct_3d_->getDevice()->SetLight( 0, light_ );
	direct_3d_->getDevice()->LightEnable( 0, TRUE );
	*/

	return true;
}

void Direct3D9Mesh::render()
{
	for ( unsigned int n = 0; n < material_count_; n++ )
	{
		direct_3d_->getDevice()->SetMaterial( & materials_[ n ] );
		// direct_3d_->getDevice()->SetTexture( 0, 0 );

		FAIL_CHECK( mesh_->DrawSubset( n ) );
	}
}