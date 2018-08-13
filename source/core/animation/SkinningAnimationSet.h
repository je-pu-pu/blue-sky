#pragma once

#include "SkinningAnimation.h"
#include <core/type.h>
#include <common/exception.h>
#include <vector>
#include <map>

namespace core
{

/**
 * �X�L�j���O�A�j���[�V�����̕����Z�b�g
 *
 */
class SkinningAnimationSet
{
public:
	typedef std::map< const string_t, SkinningAnimation > SkinningAnimationMap;
	typedef std::vector< Matrix* > MatrixList;
	typedef std::vector< uint_t > BoneIndexList;
	typedef std::vector< BoneIndexList > ChildBoneIndexList;
	typedef std::vector< uint_t > ParentBoneIndexList;

private:
	SkinningAnimationMap	skinning_animation_map_;
	MatrixList				bone_offset_matrix_list_;
	ParentBoneIndexList		parent_bone_index_list_;
	ChildBoneIndexList		child_bone_index_list_;

public:
	SkinningAnimationSet()
	{
		parent_bone_index_list_.resize( 1 );
		parent_bone_index_list_[ 0 ] = 0;
	}

	~SkinningAnimationSet()
	{
		for ( auto i = bone_offset_matrix_list_.begin(); i != bone_offset_matrix_list_.end(); ++i )
		{
			delete *i;
		}
	}

	/**
	 * �w�肵�����O�ɑΉ������X�L�j���O�A�j���[�V�������擾����
	 *
	 * @param name �X�L�j���O�A�j���[�V�����̖��O
	 * @return �X�L�j���O�A�j���[�V����
	 */
	SkinningAnimation& get_skinning_animation( const char_t* name )
	{
		auto i = skinning_animation_map_.find( name );

		if ( i == skinning_animation_map_.end() )
		{
			i = skinning_animation_map_.insert( std::make_pair( name, SkinningAnimation( bone_offset_matrix_list_.size() ) ) ).first;
		}

		return i->second;
	}

	const SkinningAnimation* get_skinning_animation( const char_t* name ) const
	{
		auto i = skinning_animation_map_.find( name );

		if ( i == skinning_animation_map_.end() )
		{
			return 0;
		}

		return & i->second;
	}


	/**
	 * �w�肵�����O�ƃ{�[���C���f�b�N�X�ɑΉ�����A�j���[�V�������擾����
	 *
	 * @param name �X�L�j���O�A�j���[�V�����̖��O
	 * @param bone_index �{�[���C���f�b�N�X
	 * @return �A�j���[�V����
	 */
	Animation& get_bone_animation( const char_t* name, uint_t bone_index )
	{
		return get_skinning_animation( name ).get_bone_animation_by_bone_index( bone_index );
	}

	/*** @todo �������� */
	Matrix& get_bone_offset_matrix_by_bone_index( uint_t bone_index ) { return *bone_offset_matrix_list_[ bone_index ]; }
	const Matrix& get_bone_offset_matrix_by_bone_index( uint_t bone_index ) const { return *bone_offset_matrix_list_[ bone_index ]; }

	/**
	 * �{�[���̐����擾����
	 *
	 * @return �{�[���̐�
	 */
	uint_t get_bone_count() const
	{
		return bone_offset_matrix_list_.size();
	}

	/**
	 * �{�[���̐���ݒ肷��
	 *
	 * @param count �{�[���̐�
	 */
	void set_bone_count( uint_t count )
	{
		if ( ! bone_offset_matrix_list_.empty() )
		{
			COMMON_THROW_EXCEPTION_MESSAGE( "duplicate call set_bonoe_count()" );
		}

		bone_offset_matrix_list_.resize( count );

		for ( auto i = bone_offset_matrix_list_.begin(); i != bone_offset_matrix_list_.end(); ++i )
		{
			*i = new Matrix( Matrix::identity() );
		}

		for ( auto i = skinning_animation_map_.begin(); i != skinning_animation_map_.end(); ++i )
		{
			i->second.resize_bone_animation_list( count );
		}

		parent_bone_index_list_.resize( count );
		child_bone_index_list_.resize( count );
	}

	/**
	 * �e�{�[���̃C���f�b�N�X�Ɏq�{�[���̃C���f�b�N�X��Ή�������
	 *
	 * @param bone_index �e�{�[���̃C���f�b�N�X
	 * @param child_bone_index �{�[���̃C���f�b�N�X
	 */
	void add_child_bone_index( uint_t bone_index, uint_t child_bone_index )
	{
		if ( bone_index >= child_bone_index_list_.size() )
		{
			child_bone_index_list_.resize( bone_index + 1 );
		}

		child_bone_index_list_[ bone_index ].push_back( child_bone_index );

		if ( child_bone_index >= parent_bone_index_list_.size() )
		{
			parent_bone_index_list_.resize( child_bone_index + 1 );
		}

		parent_bone_index_list_[ child_bone_index ] = bone_index;
	}

	/**
	 * �C���f�b�N�X�Ŏw�肵���{�[���̎q�{�[���̐����擾����
	 *
	 * @param bone_index �{�[���̃C���f�b�N�X
	 */
	uint_t get_child_bone_count( uint_t bone_index ) const
	{
		if ( bone_index >= child_bone_index_list_.size() )
		{
			return 0;
		}

		return child_bone_index_list_[ bone_index ].size();
	}

	/**
	 * �q�{�[���̃C���f�b�N�X���擾����
	 *
	 * @param bone_index �{�[���̃C���f�b�N�X
	 * @param n �q�{�[���̐e�{�[�����ł̃C���f�b�N�X
	 * @return �q�{�[���̑S�{�[�����ł̃C���f�b�N�X
	 */
	uint_t get_child_bone_index( uint_t bone_index, uint_t n ) const
	{
		return child_bone_index_list_[ bone_index ][ n ];
	}

	/**
	 * �e�{�[���̃C���f�b�N�X���擾����
	 *
	 * @param bone_index �{�[���̃C���f�b�N�X
	 * @return �e�{�[���̑S�{�[�����ł̃C���f�b�N�X
	 */
	uint_t get_parent_bone_index( uint_t bone_index ) const
	{
		return parent_bone_index_list_[ bone_index ];
	}

	/**
	 * �œK������
	 *
	 */
	void optimize()
	{
		for ( auto i = skinning_animation_map_.begin(); i != skinning_animation_map_.end(); ++i )
		{
			i->second.optimize();
		}
	}

	/**
	 * �A�j���[�V�����̒������v�Z����
	 *
	 */
	void calculate_length()
	{
		for ( auto i = skinning_animation_map_.begin(); i != skinning_animation_map_.end(); ++i )
		{
			i->second.calculate_length();
		}
	}

}; // class SkinningAnimation

} // namespace core
