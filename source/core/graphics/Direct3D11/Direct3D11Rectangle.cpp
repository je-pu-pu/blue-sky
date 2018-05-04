#include "Direct3D11Rectangle.h"
#include "Direct3D11Material.h"
#include "Direct3D11.h"

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

	auto* vertex_group = create_vertex_group();
	
	vertex_group->add_index( 0 );
	vertex_group->add_index( 1 );
	vertex_group->add_index( 2 );

	vertex_group->add_index( 2 );
	vertex_group->add_index( 1 );
	vertex_group->add_index( 3 );
	
	create_vertex_buffer();
	create_index_buffer();
}
