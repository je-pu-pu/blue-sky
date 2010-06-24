#include "Direct3D9Mesh.h"
#include "Direct3D9.h"
#include "DirectX.h"

#include <common/exception.h>
#include <common/math.h>


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
	if ( mesh_ )
	{
		mesh_->Release();
	}

	delete [] materials_;
	delete [] textures_;
}


bool Direct3D9Mesh::load_x( const char* file_name )
{
	LPD3DXBUFFER adjacency_buffer;
	LPD3DXBUFFER materials_buffer;

	DIRECT_X_FAIL_CHECK( D3DXLoadMeshFromX( file_name, D3DXMESH_SYSTEMMEM, direct_3d_->getDevice(), & adjacency_buffer, & materials_buffer, 0, & material_count_, & mesh_ ) );

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