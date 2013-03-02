#include "Direct3D11Rectangle.h"
#include "Direct3D11Material.h"
#include "Direct3D11.h"
#include "DirectX.h"

Direct3D11Rectangle::Direct3D11Rectangle( Direct3D11* direct_3d )
	: Direct3D11Mesh( direct_3d )
{	
	/**
	 * 0----1
	 * |    |
	 * |    |
	 * 2----3
	 */

	// 
	vertex_list_.resize( 4 );

	vertex_list_[ 0 ].Position = Position( -1.f, +1.f, 0.f );
	vertex_list_[ 0 ].TexCoord = TexCoord( 0.f, 0.f );

	vertex_list_[ 1 ].Position = Position( +1.f, +1.f, 0.f );
	vertex_list_[ 1 ].TexCoord = TexCoord( 1.f, 0.f );

	vertex_list_[ 2 ].Position = Position( -1.f, -1.f, 0.f );
	vertex_list_[ 2 ].TexCoord = TexCoord( 0.f, 1.f );

	vertex_list_[ 3 ].Position = Position( +1.f, -1.f, 0.f );
	vertex_list_[ 3 ].TexCoord = TexCoord( 1.f, 1.f );

	// 
	get_material_list().push_back( new Material( direct_3d_ ) );
	Material* material = * get_material_list().begin();

	material->get_index_list().resize( 3 * 2 );

	material->get_index_list()[ 0 ] = 0;
	material->get_index_list()[ 1 ] = 1;
	material->get_index_list()[ 2 ] = 2;

	material->get_index_list()[ 3 ] = 2;
	material->get_index_list()[ 4 ] = 1;
	material->get_index_list()[ 5 ] = 3;

	// 
	create_vertex_buffer();

	material->create_index_buffer();
}
