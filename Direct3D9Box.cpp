#include "Direct3D9Box.h"
#include "Direct3D9.h"
#include "DirectX.h"

#include <common/exception.h>
#include <common/math.h>

Direct3D9Box::Direct3D9Box( Direct3D9* direct_3d, float w, float h, float d, D3DCOLOR c )
	: direct_3d_( direct_3d )
{
	D3DVERTEXELEMENT9 vertex_element[] = {
		{ 0, 0, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 1, 0, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		D3DDECL_END()
	};

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateVertexDeclaration( vertex_element, & vertex_declaration_ ) );


	D3DXVECTOR3* position;

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateVertexBuffer( sizeof( D3DXVECTOR3 ) * 8, 0, 0, D3DPOOL_DEFAULT, & position_vertex_buffer_, 0 ) );
	DIRECT_X_FAIL_CHECK( position_vertex_buffer_->Lock( 0, 0, reinterpret_cast< void** >( & position ), 0 ) );

	position[ 0 ] = D3DXVECTOR3( -w / 2, -h / 2, -d / 2 );
	position[ 1 ] = D3DXVECTOR3( +w / 2, -h / 2, -d / 2 );
	position[ 2 ] = D3DXVECTOR3( -w / 2, +h / 2, -d / 2 );
	position[ 3 ] = D3DXVECTOR3( +w / 2, +h / 2, -d / 2 );
	position[ 4 ] = D3DXVECTOR3( -w / 2, -h / 2, +d / 2 );
	position[ 5 ] = D3DXVECTOR3( +w / 2, -h / 2, +d / 2 );
	position[ 6 ] = D3DXVECTOR3( -w / 2, +h / 2, +d / 2 );
	position[ 7 ] = D3DXVECTOR3( +w / 2, +h / 2, +d / 2 );

	DIRECT_X_FAIL_CHECK( position_vertex_buffer_->Unlock() );


	D3DCOLOR* color;

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateVertexBuffer( sizeof( D3DCOLOR ) * 8, 0, 0, D3DPOOL_DEFAULT, & color_vertex_buffer_, 0 ) );
	DIRECT_X_FAIL_CHECK( color_vertex_buffer_->Lock( 0, 0, reinterpret_cast< void** >( & color ), 0 ) );

	color[ 0 ] = D3DCOLOR_XRGB( 0x00, 0x00, 0x00 );
	color[ 1 ] = D3DCOLOR_XRGB( 0x00, 0x00, 0x00 );
	color[ 2 ] = c;
	color[ 3 ] = c;
	color[ 4 ] = D3DCOLOR_XRGB( 0x00, 0x00, 0x00 );
	color[ 5 ] = D3DCOLOR_XRGB( 0x00, 0x00, 0x00 );
	color[ 6 ] = c;
	color[ 7 ] = c;

	DIRECT_X_FAIL_CHECK( color_vertex_buffer_->Unlock() );

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateIndexBuffer( 36 * 2, 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, & index_buffer_, 0 ) );

	DWORD* index;

	DIRECT_X_FAIL_CHECK( index_buffer_->Lock( 0, 0, reinterpret_cast< void** >( & index ), 0 ) );
	
	/*
	  6--7
	 /| /|
	2--3 |
	| 4|-5
	|/ |/
	0--1
	*/

	WORD index_src[] = {
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
		5, 7, 6
	};

	memcpy( index, index_src, sizeof( index_src ) );

	DIRECT_X_FAIL_CHECK( index_buffer_->Unlock() );
}


Direct3D9Box::~Direct3D9Box()
{

}

void Direct3D9Box::ready()
{
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetVertexDeclaration( vertex_declaration_ ) );
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetStreamSource( 0, position_vertex_buffer_, 0, sizeof( D3DXVECTOR3 ) ) );
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetStreamSource( 1, color_vertex_buffer_, 0, sizeof( D3DCOLOR ) ) );
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetIndices( index_buffer_ ) );

	// DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE ) );
}

void Direct3D9Box::render()
{
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12 ) );
	// DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->DrawIndexedPrimitive( D3DPT_LINELIST, 0, 0, 8, 0, 3 ) );
}