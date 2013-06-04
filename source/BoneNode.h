#ifndef BONE_NODE_H
#define BONE_NODE_H

#include <vector>

/**
 * ボーンノード
 *
 */
template< typename Matrix >
class BoneNode
{
public:
	typedef std::vector< BoneNode* > BoneNodeList;

private:
	BoneNode* parent_;			///< 親ノード
	MatrixList child_list_;		///< 子ボーンノードのリスト

	int index_;					///< ボーンインデックス
	Matrix offset_matrix_;		///< ボーンオフセット行列

public:
	BoneNode( BoneNode* parent, int index, const Matrix& offset_matrix )
		: parent_( parent )
		, index_( index )
		, offset_matrix_( offset_matrix )
	{ }
	
	void add_child( BoneNode* node )
	{
		child_list_.push_back( node );
	}

	BoneNodeList& get_child_list() { return child_list_; }
	const BoneNodeList& get_child_list() const { return child_list_; }
	
	Matrix& get_offset_matrix() const { return bone_offset_matrix_; }
	const Matrix& get_offset_matrix() const { return bone_offset_matrix_; }

}; // class BoneNode

#endif // BONE_NODE_H
