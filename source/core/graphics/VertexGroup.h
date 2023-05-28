#pragma once

#include <type/type.h>
#include <vector>

namespace core::graphics
{

/**
 * ���_�O���[�v ( ���_�C���f�b�N�X�̃��X�g )
 *
 */
class VertexGroup
{
public:
	using Index		= std::uint16_t;
	using IndexList	= std::vector< Index >;

private:
	IndexList index_list_;

public:
	VertexGroup() { }
	~VertexGroup() { }

	void clear() { index_list_.clear(); }

	void add_index( Index i ) { index_list_.push_back( i ); }
	void set_index( std::initializer_list< Index > is ) { index_list_ = is; }
	
	Index* get_pointer() { return & index_list_[ 0 ]; }
	const Index* get_pointer() const { return & index_list_[ 0 ]; }

	bool empty() const { return index_list_.empty(); }
	IndexList::size_type size() { return index_list_.size(); }

	void flip_polygon()
	{
		for ( IndexList::size_type n = 0; n < index_list_.size(); n += 3 )
		{
			std::swap( index_list_[ n ], index_list_[ n + 1 ] );
		}
	}
};

} // namespace core::graphics
