#include "Direct3D9Mesh.h"
#include "Direct3D9.h"
#include "DirectX.h"

#include <common/exception.h>
#include <common/math.h>


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

/// @todo ‚È‚ñ‚Æ‚©‚·‚é
D3DMATERIAL9* materials_ = 0;
LPDIRECT3DTEXTURE9* textures_ = 0;

bool Direct3D9Mesh::loadX( const char* file_name )
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
		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetMaterial( & materials_[ n ] ) );
		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetTexture( 0, textures_[ n ] ) );

		DIRECT_X_FAIL_CHECK( mesh_->DrawSubset( n ) );
	}
}