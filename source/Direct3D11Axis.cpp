#include "Direct3D11Axis.h"
#include "Direct3D11.h"
#include "DirectX.h"

Direct3D11Axis::Direct3D11Axis( Direct3D11* direct_3d )
	: direct_3d_( direct_3d )
	, vertex_buffer_( 0 )
{
	create_vertex_buffer();
}

Direct3D11Axis::~Direct3D11Axis()
{
	DIRECT_X_RELEASE( vertex_buffer_ );
}

void Direct3D11Axis::create_vertex_buffer()
{
	Vertex vertex_list[ 6 ] = {
		{ Position( 0.f, 0.f, 0.f ), Color( 1.f, 0.f, 0.f ) },
		{ Position( 1.f, 0.f, 0.f ), Color( 1.f, 0.f, 0.f ) },
		{ Position( 0.f, 0.f, 0.f ), Color( 0.f, 1.f, 0.f ) },
		{ Position( 0.f, 1.f, 0.f ), Color( 0.f, 1.f, 0.f ) },
		{ Position( 0.f, 0.f, 0.f ), Color( 0.f, 0.f, 1.f ) },
		{ Position( 0.f, 0.f, 1.f ), Color( 0.f, 0.f, 1.f ) },
	};

	D3D11_BUFFER_DESC buffer_desc = { 0 };

	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.ByteWidth = sizeof( Vertex ) * 6;
    
	D3D11_SUBRESOURCE_DATA data = { 0 };
	data.pSysMem = & vertex_list;
	
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, & data, & vertex_buffer_ ) );
}

void Direct3D11Axis::render() const
{
	UINT stride = sizeof( Vertex );
    UINT offset = 0;

	direct_3d_->getImmediateContext()->IASetVertexBuffers( 0, 1, & vertex_buffer_, & stride, & offset );
	direct_3d_->getImmediateContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );
	direct_3d_->getImmediateContext()->Draw( 6, 0 );
}