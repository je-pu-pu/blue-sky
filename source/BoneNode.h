#ifndef BONE_NODE_H
#define BONE_NODE_H

#include <vector>

/**
 * �{�[���m�[�h
 *
 */
template< typename Matrix >
class BoneNode
{
public:
	typedef std::vector< BoneNode* > BoneNodeList;

private:
	BoneNode* parent_;			///< �e�m�[�h
	MatrixList child_list_;		///< �q�{�[���m�[�h�̃��X�g

	int index_;					///< �{�[���C���f�b�N�X
	Matrix offset_matrix_;		///< �{�[���I�t�Z�b�g�s��

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
