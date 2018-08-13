#pragma once

#include "Animation.h"

namespace core
{

/**
 * �X�L�j���O�A�j���[�V����
 *
 */
class SkinningAnimation
{
public:
	typedef std::vector< Animation > AnimationList;

	typedef AnimationChannel Channel;
	typedef AnimationKeyFrame KeyFrame;
	typedef Channel::KeyFrameList KeyFrameList;

private:
	AnimationList	bone_animation_list_;					///< �{�[�����̃A�j���[�V�����̈ꗗ
	float_t			length_;								///< �A�j���[�V�����̒��� ( �t���[�� )

public:
	SkinningAnimation( uint_t size )
		: bone_animation_list_( size )
		, length_( 0.f )
	{

	}

	/**
	 * �w�肵���{�[���C���f�b�N�X�ɑΉ�����A�j���[�V�������擾����
	 *
	 * @param bone_index �{�[���C���f�b�N�X
	 * @return �A�j���[�V����
	 */
	Animation& get_bone_animation_by_bone_index( uint_t bone_index )
	{
		if ( bone_index >= bone_animation_list_.size() )
		{
			bone_animation_list_.resize( bone_index + 1 );
		}

		return bone_animation_list_[ bone_index ];
	}

	const Animation& get_bone_animation_by_bone_index( uint_t bone_index ) const
	{
		return bone_animation_list_[ bone_index ];
	}

	/**
	 * �{�[�����̃A�j���[�V�����̃T�C�Y���擾����
	 *
	 * @return �{�[�����̃A�j���[�V�����̈ꗗ�̃T�C�Y
	 */
	uint_t get_bone_animation_list_size() const
	{
		return bone_animation_list_.size();
	}

	/**
	 * �{�[�����̃A�j���[�V�����̈ꗗ�����T�C�Y����
	 *
	 * @param size �{�[�����̃A�j���[�V�����̈ꗗ�̃T�C�Y
	 */
	void resize_bone_animation_list( uint_t size )
	{
		bone_animation_list_.resize( size );
	}

	/**
	 * �œK������
	 *
	 */
	void optimize()
	{
		for ( auto i = bone_animation_list_.begin(); i != bone_animation_list_.end(); ++i )
		{
			i->optimize();
		}
	}

	/**
	 * �A�j���[�V�����̒������v�Z����
	 *
	 */
	float_t calculate_length()
	{
		for ( auto i = bone_animation_list_.begin(); i != bone_animation_list_.end(); ++i )
		{
			length_ = std::max( length_, i->calculate_length() );
		}

		return length_;
	}

	/**
	 * �A�j���[�V�����̒������擾����
	 *
	 */
	float_t get_length() const
	{
		return length_;
	}

}; // class SkinningAnimation

} // namespace core
