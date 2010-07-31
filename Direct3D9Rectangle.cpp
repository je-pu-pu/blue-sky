#include "Direct3D9Rectangle.h"
#include "Direct3D9.h"
#include "DirectX.h"

#include <common/exception.h>
#include <common/math.h>

Direct3D9Rectangle::Direct3D9Rectangle( Direct3D9* direct_3d )
	: direct_3d_( direct_3d )
{
	D3DVERTEXELEMENT9 vertex_element[] = {
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 1, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateVertexDeclaration( vertex_element, & vertex_declaration_ ) );


	D3DXVECTOR3* position;

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateVertexBuffer( sizeof( D3DXVECTOR3 ) * 4, 0, 0, D3DPOOL_MANAGED, & position_vertex_buffer_, 0 ) );
	DIRECT_X_FAIL_CHECK( position_vertex_buffer_->Lock( 0, 0, reinterpret_cast< void** >( & position ), 0 ) );

	position[ 0 ] = D3DXVECTOR3( -1.f, +1.f, 0.f );
	position[ 1 ] = D3DXVECTOR3( +1.f, +1.f, 0.f );
	position[ 2 ] = D3DXVECTOR3( -1.f, -1.f, 0.f );
	position[ 3 ] = D3DXVECTOR3( +1.f, -1.f, 0.f );

	DIRECT_X_FAIL_CHECK( position_vertex_buffer_->Unlock() );

	D3DXVECTOR2* uv;

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateVertexBuffer( sizeof( D3DXVECTOR2 ) * 4, 0, 0, D3DPOOL_MANAGED, & uv_vertex_buffer_, 0 ) );
	DIRECT_X_FAIL_CHECK( uv_vertex_buffer_->Lock( 0, 0, reinterpret_cast< void** >( & uv ), 0 ) );

	uv[ 0 ] = D3DXVECTOR2( 0.f, 1.f );
	uv[ 1 ] = D3DXVECTOR2( 1.f, 1.f );
	uv[ 2 ] = D3DXVECTOR2( 0.f, 0.f );
	uv[ 3 ] = D3DXVECTOR2( 1.f, 0.f );

	DIRECT_X_FAIL_CHECK( uv_vertex_buffer_->Unlock() );

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateIndexBuffer( 4 * 2, 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, & index_buffer_, 0 ) );

	DWORD* index;

	DIRECT_X_FAIL_CHECK( index_buffer_->Lock( 0, 0, reinterpret_cast< void** >( & index ), 0 ) );
	
	/**
	 * 0----1
	 * |    |
	 * |    |
	 * 2----3
	 */
	WORD index_src[] = { 0, 1, 2, 3 };

	memcpy( index, index_src, sizeof( index_src ) );

	DIRECT_X_FAIL_CHECK( index_buffer_->Unlock() );
}


Direct3D9Rectangle::~Direct3D9Rectangle()
{
	vertex_declaration_->Release();
	position_vertex_buffer_->Release();
	uv_vertex_buffer_->Release();
	index_buffer_->Release();
}

void Direct3D9Rectangle::ready()
{
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetVertexDeclaration( vertex_declaration_ ) );
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetStreamSource( 0, position_vertex_buffer_, 0, sizeof( D3DXVECTOR3 ) ) );
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetStreamSource( 1, uv_vertex_buffer_, 0, sizeof( D3DXVECTOR2 ) ) );
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetIndices( index_buffer_ ) );

	// DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE ) );
}

void Direct3D9Rectangle::render()
{
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP, 0, 0, 4, 0, 2 ) );
}