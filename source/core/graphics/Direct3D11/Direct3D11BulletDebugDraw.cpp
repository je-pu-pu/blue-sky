#include "Direct3D11BulletDebugDraw.h"
#include "Direct3D11.h"

#include <common/exception.h>
#include <common/math.h>

// #include <boost/filesystem.hpp>

#include <map>

#include <fstream>
#include <sstream>

Direct3D11BulletDebugDraw::Direct3D11BulletDebugDraw( Direct3D11* direct_3d )
	: direct_3d_( direct_3d )
	, vertex_buffer_( 0 )
	, debug_mode_( 0 )
{

}

Direct3D11BulletDebugDraw::~Direct3D11BulletDebugDraw()
{
	DIRECT_X_RELEASE( vertex_buffer_ );
}

void Direct3D11BulletDebugDraw::create_vertex_buffer()
{
	if ( vertex_list_.empty() )
	{
		return;
	}

	// !!!
	DIRECT_X_RELEASE( vertex_buffer_ );

	D3D11_BUFFER_DESC buffer_desc = { 0 };

	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.ByteWidth = sizeof( Vertex ) * vertex_list_.size();
    
	D3D11_SUBRESOURCE_DATA data = { 0 };
	data.pSysMem = & vertex_list_[ 0 ];
	
	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, & data, & vertex_buffer_ ) );
}

void Direct3D11BulletDebugDraw::drawLine( const btVector3& from, const btVector3& to, const btVector3& color )
{
	Vertex from_v;
	Vertex to_v;

	from_v.Position = Position( from.x(), from.y(), from.z() );
	from_v.Color = Color( color.x(), color.y(), color.z() );

	to_v.Position = Position( to.x(), to.y(), to.z() );
	to_v.Color = Color( color.x(), color.y(), color.z() );

	vertex_list_.push_back( from_v );
	vertex_list_.push_back( to_v );
}

void Direct3D11BulletDebugDraw::clear()
{
	vertex_list_.clear();
}

void Direct3D11BulletDebugDraw::render() const
{
	if ( vertex_list_.empty() )
	{
		return;
	}

	const_cast< Direct3D11BulletDebugDraw* >( this )->create_vertex_buffer();

	UINT stride = sizeof( Vertex );
    UINT offset = 0;

	direct_3d_->getImmediateContext()->IASetVertexBuffers( 0, 1, & vertex_buffer_, & stride, & offset );
	direct_3d_->getImmediateContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );
	direct_3d_->getImmediateContext()->Draw( vertex_list_.size(), 0 );
}