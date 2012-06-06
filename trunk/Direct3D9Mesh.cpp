#include "Direct3D9Mesh.h"
#include "Direct3D9.h"
#include "Direct3D9TextureManager.h"
#include "DirectX.h"

#include <common/exception.h>
#include <common/math.h>

#include <dxfile.h>

#include <boost/algorithm/string.hpp>

#include <vector>


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

	if ( FAILED( D3DXLoadMeshFromX( file_name, D3DXMESH_MANAGED, direct_3d_->getDevice(), & adjacency_buffer, & materials_buffer, 0, & material_count_, & mesh_ ) ) )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( std::string( "load x file " ) + file_name + " failed." );
	}
	// DIRECT_X_FAIL_CHECK( D3DXLoadMeshFromX( file_name, D3DXMESH_SYSTEMMEM, direct_3d_->getDevice(), & adjacency_buffer, & materials_buffer, 0, & material_count_, & mesh_ ) );

	// DIRECT_X_FAIL_CHECK( D3DXSaveMeshToX( ( std::string( file_name ) + ".new.x" ).c_str(), mesh_, static_cast< DWORD* >( adjacency_buffer->GetBufferPointer() ), static_cast< D3DXMATERIAL* >( materials_buffer->GetBufferPointer() ), 0, material_count_, DXFILEFORMAT_TEXT ) );

	// mesh_->GetOptions()

	DIRECT_X_FAIL_CHECK( mesh_->OptimizeInplace( D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE, static_cast< DWORD* >( adjacency_buffer->GetBufferPointer() ), 0, 0, 0 ) );

	materials_ = new D3DMATERIAL9[ material_count_ ];
	textures_ = new LPDIRECT3DTEXTURE9[ material_count_ ];

	for ( unsigned int n = 0; n < material_count_; n++ )
	{
		D3DXMATERIAL* material = & static_cast< D3DXMATERIAL* >( materials_buffer->GetBufferPointer() )[ n ];

		materials_[ n ] = material->MatD3D;
		materials_[ n ].Ambient = materials_[ n ].Diffuse;

		textures_[ n ] = load_texture( material->pTextureFilename );
	}

	// mesh_->LockVertexBuffer( 

	adjacency_buffer->Release();
	materials_buffer->Release();

	D3DVERTEXELEMENT9 vertex_element[] =
	{
		{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	LPD3DXMESH new_mesh = 0;
	DIRECT_X_FAIL_CHECK( mesh_->CloneMesh( D3DXMESH_MANAGED, vertex_element, direct_3d_->getDevice(), & new_mesh ) );
	mesh_->Release();
	mesh_ = new_mesh;

	return true;
}

LPDIRECT3DTEXTURE9 Direct3D9Mesh::load_texture( const char* texture_name ) const
{
	if ( ! texture_name )
	{
		return 0;
	}

	std::string texture_file_name = get_texture_file_name_by_texture_name( texture_name );

	std::vector< std::string > name_list;
	boost::algorithm::split( name_list, texture_file_name, boost::algorithm::is_any_of( std::string( "\\/" ) ) );

	return direct_3d_->getTextureManager()->load( name_list[ name_list.size() - 1 ].c_str(), get_texture_file_name_by_texture_name( texture_name ).c_str() );
}

std::string Direct3D9Mesh::get_texture_file_name_by_texture_name( const char* texture_name ) const
{
	return std::string( "media/model/" ) + texture_name;
}

void Direct3D9Mesh::render() const
{
	for ( unsigned int n = 0; n < material_count_; n++ )
	{
		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetMaterial( & materials_[ n ] ) );
		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetTexture( 0, textures_[ n ] ) );

		DIRECT_X_FAIL_CHECK( mesh_->DrawSubset( n ) );
	}
}