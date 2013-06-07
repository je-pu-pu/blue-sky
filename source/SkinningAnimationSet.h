#ifndef SKINNING_ANIMATION_SET_H
#define SKINNING_ANIMATION_SET_H

#include "type.h"
#include "SkinningAnimation.h"

/**
 * �X�L�j���O�A�j���[�V�����̕����Z�b�g
 *
 */
class SkinningAnimationSet
{
public:
	typedef std::map< const string_t, SkinningAnimation > SkinningAnimationMap;
	typedef std::vector< Matrix > MatrixList;

private:
	SkinningAnimationMap	skinning_animation_map_;
	MatrixList				bone_offset_matrix_list_;

public:
	SkinningAnimationSet()
	{ }

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
	Matrix& get_bone_offset_matrix_by_bone_index( uint_t bone_index ) { return bone_offset_matrix_list_[ bone_index ]; }
	const Matrix& get_bone_offset_matrix_by_bone_index( uint_t bone_index ) const { return bone_offset_matrix_list_[ bone_index ]; }

	/**
	 * �{�[���̐���ݒ肷��
	 *
	 * @param count �{�[���̐�
	 */
	void set_bone_count( uint_t count )
	{
		bone_offset_matrix_list_.resize( count );

		for ( auto i = skinning_animation_map_.begin(); i != skinning_animation_map_.end(); ++i )
		{
			i->second.resize_bone_animation_list( count );
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

#endif // SKINNING_ANIMATION_SET_H
