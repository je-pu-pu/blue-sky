#include "Direct3D9Mesh.h"
#include "Direct3D9.h"
#include "DirectX.h"

#include <common/exception.h>
#include <common/math.h>

#include <dxfile.h>


Direct3D9Mesh::Direct3D9Mesh( Direct3D9* direct_3d )
	: direct_3d_( direct_3d )
	, mesh_( 0 )
	, material_count_( 0 )
	, materials_( 0 )
	, textures_( 0 )
{

}

Direct3D9Mesh::~Direct3D9Mesh()
{
	for ( unsigned int n = 0; n < material_count_; n++ )
	{
		if ( textures_[ n ] )
		{
			textures_[ n ]->Release();
		}
	}

	delete [] materials_;
	delete [] textures_;

	if ( mesh_ )
	{
		mesh_->Release();
	}
}


bool Direct3D9Mesh::load_x( const char* file_name )
{
	LPD3DXBUFFER adjacency_buffer;
	LPD3DXBUFFER materials_buffer;

	DIRECT_X_FAIL_CHECK( D3DXLoadMeshFromX( file_name, D3DXMESH_SYSTEMMEM, direct_3d_->getDevice(), & adjacency_buffer, & materials_buffer, 0, & material_count_, & mesh_ ) );

	// DIRECT_X_FAIL_CHECK( D3DXSaveMeshToX( ( std::string( file_name ) + ".new.x" ).c_str(), mesh_, static_cast< DWORD* >( adjacency_buffer->GetBufferPointer() ), static_cast< D3DXMATERIAL* >( materials_buffer->GetBufferPointer() ), 0, material_count_, DXFILEFORMAT_TEXT ) );

	// mesh_->GetOptions()

	mesh_->OptimizeInplace( D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE, static_cast< DWORD* >( adjacency_buffer->GetBufferPointer() ), 0, 0, 0 );

	materials_ = new D3DMATERIAL9[ material_count_ ];
	textures_ = new LPDIRECT3DTEXTURE9[ material_count_ ];

	for ( unsigned int n = 0; n < material_count_; n++ )
	{
		D3DXMATERIAL* material = & static_cast< D3DXMATERIAL* >( materials_buffer->GetBufferPointer() )[ n ];

		materials_[ n ] = material->MatD3D;
		materials_[ n ].Ambient = materials_[ n ].Diffuse;

		textures_[ n ] = 0;

		if ( material->pTextureFilename )
		{
			try
			{
				DIRECT_X_FAIL_CHECK( D3DXCreateTextureFromFile( direct_3d_->getDevice(), ( std::string( "media/model/" ) + material->pTextureFilename ).c_str(), & textures_[ n ] ) );
			}
			catch ( ... )
			{
			}
		}
	}

	// mesh_->LockVertexBuffer( 

	adjacency_buffer->Release();
	materials_buffer->Release();

	return true;
}

void Direct3D9Mesh::render()
{
	for ( unsigned int n = 0; n < material_count_; n++ )
	{
		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetMaterial( & materials_[ n ] ) );
		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetTexture( 0, textures_[ n ] ) );

		DIRECT_X_FAIL_CHECK( mesh_->DrawSubset( n ) );
	}
}