#pragma once

#include <core/graphics/GraphicsManager.h>
#include <core/graphics/VertexBuffer.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <vector>

namespace core::graphics
{

/**
 * 点の列を動的に描く
 *
 * 点は CPU 側の一覧に持ち、変わったときだけ描く前に頂点の置き場（VertexBuffer）へ書く。
 * 以前は Direct3D 11 の実装そのもので、使う側（シーン）が Direct3D 11 を受け取って作っていた
 */
template< typename VertexType >
class DynamicPointList
{
public:
	const unsigned int MAX_VERTEX_COUNT = 200 * 200 * 200;

	using VertexList	= std::vector< VertexType >;

private:
	const GraphicsManager*			graphics_manager_;
	std::unique_ptr< VertexBuffer >	vertex_buffer_;
	VertexList						vertex_list_;

	bool changed_;

public:
	/// graphics_manager は頂点の置き場を作り、描く形を決める先（作り手が渡す）
	explicit DynamicPointList( GraphicsManager* graphics_manager )
		: graphics_manager_( graphics_manager )
		, vertex_buffer_( graphics_manager->create_dynamic_vertex_buffer( sizeof( VertexType ), MAX_VERTEX_COUNT ) )
		, changed_( false )
	{
		vertex_list_.reserve( MAX_VERTEX_COUNT );
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
			vertex_buffer_->update( vertex_list_.data(), vertex_list_.size() );

			changed_ = false;
		}

		graphics_manager_->set_primitive_topology( PrimitiveTopology::POINT_LIST );
		vertex_buffer_->draw( vertex_list_.size() );
	}

	size_t size() const { return vertex_list_.size(); }
};

} // namespace core::graphics
