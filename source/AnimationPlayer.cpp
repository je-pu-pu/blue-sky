#include "AnimationPlayer.h"

#include "Direct3D11Matrix.h"
#include "GameMain.h"

#include "memory.h"

/**
 * �R���X�g���N�^
 *
 */
AnimationPlayer::AnimationPlayer( const SkinningAnimationSet* skinning_animation_set )
	: skinning_animation_set_( skinning_animation_set )
	, current_skinning_animation_( 0 )
	, current_frame_( 0.f )
	, is_looping_( false )
	, constant_buffer_( blue_sky::GameMain::get_instance()->get_direct_3d() )
{
	
}

/**
 * �A�j���[�V�������Đ�����
 *
 * @param name �A�j���[�V������
 * @param force �����I�ɍŏ�����Đ�����t���O
 * @param loop ���[�v�Đ��t���O
 */
void AnimationPlayer::play( const char_t* name, bool force, bool loop )
{
	const SkinningAnimation* skinning_animation = skinning_animation_set_->get_skinning_animation( name );

	if ( ! skinning_animation )
	{
		return;
	}

	is_looping_ = loop;

	if ( skinning_animation == current_skinning_animation_ && ! force )
	{
		return;
	}

	current_skinning_animation_ = skinning_animation;
	current_frame_ = 0.f;
}

/**
 * ���ݍĐ����̃A�j���[�V�������~����
 *
 */
void AnimationPlayer::stop()
{
	current_skinning_animation_ = nullptr;
	current_frame_ = 0.f;
}

/**
 * �A�j���[�V�������X�V����
 *
 */
void AnimationPlayer::update()
{
	if ( ! current_skinning_animation_ )
	{
		return;
	}

	current_frame_ += 1.f;

	if ( is_looping_ )
	{
		if ( current_frame_ > current_skinning_animation_->get_length() )
		{
			current_frame_ = 0.f;
		}
	}
}

/**
 * �`��p�̃f�[�^���X�V����
 *
 * @todo �K�v�Œ���̃{�[���s�񂾂��X�V����悤�ɂ���
 */
void AnimationPlayer::update_render_data() const
{
	if ( ! get_current_skinning_animation() )
	{
		blue_sky::BoneConstantBufferData data;
		
		for ( uint_t n = 0; n < get_skinning_animation_set()->get_bone_count(); ++n )
		{
			data.bone_matrix[ n ].identity();
		}

		constant_buffer_.update( & data );

		return;
	}

	blue_sky::BoneConstantBufferData data;

	calculate_bone_matrix_recursive( data, 0, Matrix::identity() );

	for ( uint_t n = 0; n < get_skinning_animation_set()->get_bone_count(); ++n )
	{
		data.bone_matrix[ n ] = data.bone_matrix[ n ].transpose();
	}

	constant_buffer_.update( & data );
}

/**
 * �{�[���}�g���b�N�X���ċA�I�Ɍv�Z����
 *
 * @param data �{�[���}�g���b�N�X
 * @param bone_index �{�[���̃C���f�b�N�X
 * @param parent_bone_matrix �e�{�[���s��
 */
void AnimationPlayer::calculate_bone_matrix_recursive( BoneConstantBuffer::Data& data, uint_t bone_index, const Matrix& parent_bone_matrix ) const
{
	const Matrix& bone_offset_matrix = get_skinning_animation_set()->get_bone_offset_matrix_by_bone_index( bone_index );

	uint_t parent_bone_index = get_skinning_animation_set()->get_parent_bone_index( bone_index );
	const Matrix& parent_bone_offset_matrix = bone_index == 0 ? Matrix::identity() : get_skinning_animation_set()->get_bone_offset_matrix_by_bone_index( parent_bone_index );

	data.bone_matrix[ bone_index ] = bone_offset_matrix.inverse() * get_bone_local_matrix( bone_index ) * parent_bone_offset_matrix * parent_bone_matrix;

	for ( uint_t n = 0; n < get_skinning_animation_set()->get_child_bone_count( bone_index ); ++n )
	{
		calculate_bone_matrix_recursive( data, get_skinning_animation_set()->get_child_bone_index( bone_index, n ), data.bone_matrix[ bone_index ] );
	}
}

/**
 * �w�肵���C���f�b�N�X�̃{�[���̃��[�J���}�g���b�N�X���擾����
 *
 * @param bone_index �{�[���̃C���f�b�N�X
 * @return �{�[���̃��[�J���}�g���b�N�X
 */
Matrix AnimationPlayer::get_bone_local_matrix( uint_t bone_index ) const
{
	const Animation& bone_animation = get_current_skinning_animation()->get_bone_animation_by_bone_index( bone_index );

	Matrix /* s, */ rt;

	/*
	s.set_scaling(
		bone_animation.get_value( Animation::CHANNEL_INDEX_SX, current_frame_, 1.f ),
		bone_animation.get_value( Animation::CHANNEL_INDEX_SY, current_frame_, 1.f ),
		bone_animation.get_value( Animation::CHANNEL_INDEX_SZ, current_frame_, 1.f ) );
	*/

	rt.set_rotation_xyz(
		bone_animation.get_value( Animation::CHANNEL_INDEX_RX, current_frame_, 0.f ),
		bone_animation.get_value( Animation::CHANNEL_INDEX_RY, current_frame_, 0.f ),
		bone_animation.get_value( Animation::CHANNEL_INDEX_RZ, current_frame_, 0.f ) );

	rt.add_translation(
		bone_animation.get_value( Animation::CHANNEL_INDEX_TX, current_frame_, 0.f ),
		bone_animation.get_value( Animation::CHANNEL_INDEX_TY, current_frame_, 0.f ),
		bone_animation.get_value( Animation::CHANNEL_INDEX_TZ, current_frame_, 0.f ) );

	return /* s * */ rt;
}

/**
 * �`��p�̃f�[�^���V�F�[�_�[�Ƀo�C���h����
 *
 */
void AnimationPlayer::bind_render_data() const
{
	constant_buffer_.bind_to_vs();
}
