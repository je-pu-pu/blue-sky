#pragma once

#include <core/DirectX.h>
#include <d3d11.h>
#include <vector>

template< typename VertexType >
class DynamicPointList
{
public:
	const unsigned int MAX_VERTEX_COUNT = 160000;

	using Direct3D		= core::graphics::direct_3d_11::Direct3D11;
	using Buffer		= ID3D11Buffer;

	using VertexList	= std::vector< VertexType >;

private:
	Direct3D*			direct_3d_;
	com_ptr< Buffer >	vertex_buffer_;
	VertexList			vertex_list_;

	bool changed_;

public:
	DynamicPointList( Direct3D* direct_3d = Direct3D::get_instance() )
		: direct_3d_( direct_3d )
		, changed_( false )
	{
		create_vertex_buffer();
	}

	void DynamicPointList::create_vertex_buffer()
	{
		D3D11_BUFFER_DESC buffer_desc = { 0 };

		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffer_desc.ByteWidth = sizeof( VertexType ) * MAX_VERTEX_COUNT;
		buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
		buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, 0, & vertex_buffer_ ) );
	}
	
	void add_point( const VertexType& v, std::function< bool ( const VertexType&, const VertexType& ) > comp )
	{
		if ( vertex_list_.size() >= MAX_VERTEX_COUNT )
		{
			return;
		}

		if ( vertex_list_.size() >= 2 && comp( vertex_list_[ vertex_list_.size() - 2 ], v ) )
		{
			return;
		}

		vertex_list_.push_back( v );
		changed_ = true;
	}

	void update_last_point( const VertexType& v, std::function< bool( const VertexType&, const VertexType& ) > comp )
	{
		if ( vertex_list_.empty() )
		{
			add_point( v, comp );

			return;
		}

		vertex_list_.back() = v;;
		changed_ = true;
	}

	void erase_point( std::function< bool( const VertexType& ) > comp )
	{
		vertex_list_.erase( std::remove_if( vertex_list_.begin(), vertex_list_.end() - 1, comp ), vertex_list_.end() - 1 );
	}

	void clear()
	{
		vertex_list_.clear();
		changed_ = true;
	}

	void render()
	{
		if ( changed_ )
		{
			D3D11_MAPPED_SUBRESOURCE mapped_subresource;

			DIRECT_X_FAIL_CHECK( direct_3d_->getImmediateContext()->Map( vertex_buffer_.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, & mapped_subresource ) );

			memcpy( mapped_subresource.pData, &vertex_list_[ 0 ], sizeof( VertexType ) * vertex_list_.size() );

			direct_3d_->getImmediateContext()->Unmap( vertex_buffer_.get(), 0 );

			changed_ = false;
		}

		UINT stride[] = { sizeof( VertexType ) };
		UINT offset[] = { 0 };

		direct_3d_->getImmediateContext()->IASetVertexBuffers( 0, 1, & vertex_buffer_, stride, offset );
		direct_3d_->getImmediateContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );

		direct_3d_->getImmediateContext()->Draw( vertex_list_.size(), 0 );
	}

	size_t size() const { return vertex_list_.size(); }
};