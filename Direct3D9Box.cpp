#include "Direct3D9Box.h"
#include "Direct3D9.h"

#include <common/exception.h>
#include <common/math.h>

Direct3D9Box::Direct3D9Box( Direct3D9* direct_3d, float w, float h, float d )
	: direct_3d_( direct_3d )
{
	D3DXVECTOR3* position;

	FAIL_CHECK( direct_3d_->getDevice()->CreateVertexBuffer( sizeof( D3DXVECTOR3 ) * 8, 0, 0, D3DPOOL_DEFAULT, & position_vertex_buffer_, 0 ) );
	FAIL_CHECK( position_vertex_buffer_->Lock( 0, 0, reinterpret_cast< void** >( & position ), 0 ) );

	position[ 0 ] = D3DXVECTOR3( -w / 2, -h / 2, -d / 2 );
	position[ 1 ] = D3DXVECTOR3( +w / 2, -h / 2, -d / 2 );
	position[ 2 ] = D3DXVECTOR3( -w / 2, +h / 2, -d / 2 );
	position[ 3 ] = D3DXVECTOR3( +w / 2, +h / 2, -d / 2 );
	position[ 4 ] = D3DXVECTOR3( -w / 2, -h / 2, +d / 2 );
	position[ 5 ] = D3DXVECTOR3( +w / 2, -h / 2, +d / 2 );
	position[ 6 ] = D3DXVECTOR3( -w / 2, +h / 2, +d / 2 );
	position[ 7 ] = D3DXVECTOR3( +w / 2, +h / 2, +d / 2 );

	FAIL_CHECK( position_vertex_buffer_->Unlock() );


	D3DCOLOR* color;

	FAIL_CHECK( direct_3d_->getDevice()->CreateVertexBuffer( sizeof( D3DCOLOR ) * 8, 0, 0, D3DPOOL_DEFAULT, & color_vertex_buffer_, 0 ) );
	FAIL_CHECK( color_vertex_buffer_->Lock( 0, 0, reinterpret_cast< void** >( & color ), 0 ) );

	color[ 0 ] = D3DCOLOR_XRGB( 0x00, 0x00, 0x00 );
	color[ 1 ] = D3DCOLOR_XRGB( 0xFF, 0x00, 0x00 );
	color[ 2 ] = D3DCOLOR_XRGB( 0x00, 0xFF, 0x00 );
	color[ 3 ] = D3DCOLOR_XRGB( 0x00, 0x00, 0xFF );
	color[ 4 ] = D3DCOLOR_XRGB( 0xFF, 0xFF, 0x00 );
	color[ 5 ] = D3DCOLOR_XRGB( 0xFF, 0x00, 0xFF );
	color[ 6 ] = D3DCOLOR_XRGB( 0x00, 0xFF, 0xFF );
	color[ 7 ] = D3DCOLOR_XRGB( 0xFF, 0xFF, 0xFF );

	FAIL_CHECK( color_vertex_buffer_->Unlock() );

	FAIL_CHECK( direct_3d_->getDevice()->CreateIndexBuffer( 36 * 2, 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, & index_buffer_, 0 ) );

	DWORD* index;

	FAIL_CHECK( index_buffer_->Lock( 0, 0, reinterpret_cast< void** >( & index ), 0 ) );
	
	/*
	  6--7
	 /  /|
	2--3 |
	| 4|-5
	|/ |/
	0--1
	*/

	DWORD index_src[] = {
		0, 3, 1,
		0, 2, 3,
		
		1, 7, 5,
		1, 3, 7,

		4, 2, 0,
		4, 6, 2,

		2, 7, 3,
		2, 6, 7,

		4, 1, 5,
		4, 0, 1,

		5, 6, 4,
		5, 7, 4
	};

	memcpy( index, index_src, sizeof( index_src ) );

	FAIL_CHECK( index_buffer_->Unlock() );
}


Direct3D9Box::~Direct3D9Box()
{

}

void Direct3D9Box::ready()
{
	direct_3d_->getDevice()->SetStreamSource( 0, position_vertex_buffer_, 0, sizeof( D3DXVECTOR3 ) );
	direct_3d_->getDevice()->SetStreamSource( 1, color_vertex_buffer_, 0, sizeof( D3DCOLOR ) );
	direct_3d_->getDevice()->SetIndices( index_buffer_ );
}

void Direct3D9Box::render()
{
	FAIL_CHECK( direct_3d_->getDevice()->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12 ) );
}