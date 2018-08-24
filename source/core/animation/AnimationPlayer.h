#pragma once

#include "SkinningAnimationSet.h"
#include <blue_sky/ShaderResources.h>

namespace core
{

/**
 * �X�L�j���O�A�j���[�V�����̍Đ�
 *
 */
class AnimationPlayer
{
public:
	/// @todo ��������
	using BoneShaderResource = blue_sky::BoneShaderResource;

	static const int MaxBones = blue_sky::BoneShaderResourceData::MaxBones;

private:
	const SkinningAnimationSet* skinning_animation_set_;	///< �X�L�j���O�A�j���[�V�����̃Z�b�g
	const SkinningAnimation* current_skinning_animation_;	///< ���݂̃X�L�j���O�A�j���[�V����
	float_t	speed_;											///< �Đ��X�s�[�h
	float_t current_frame_;									///< ���݂̍Đ��t���[��
	bool is_looping_;										///< ���[�v�Đ����t���O
	bool is_broken_;										///< �j��t���O

	/// @todo �ړ�����
	const BoneShaderResource constant_buffer_;				///< �萔�o�b�t�@

	Matrix get_bone_local_matrix( uint_t ) const;

	AnimationPlayer& operator = ( const AnimationPlayer& ) { }

public:
	AnimationPlayer( const SkinningAnimationSet* );
	~AnimationPlayer() { }

	void set_speed( float_t speed ) { speed_ = speed; }
	float_t get_speed() const { return speed_; }
	void reset_speed() { speed_ = 1.f; }

	void play( const char_t* name, bool force, bool loop );
	void stop();

	void update();

	const SkinningAnimationSet* get_skinning_animation_set() const { return skinning_animation_set_; }
	const SkinningAnimation* get_current_skinning_animation() const { return current_skinning_animation_; }

	float_t get_current_frame() const { return current_frame_; }

	void calculate_bone_matrix_recursive( BoneShaderResource::Data&, uint_t, const Matrix& ) const;

	void update_render_data() const;
	void bind_render_data() const;

	const BoneShaderResource* get_shader_resource() const { return & constant_buffer_; }

	bool is_broken() const { return is_broken_; }
	void set_broken( bool b ) { is_broken_ = b; }

}; // class AnimationPlayer

} // namespace core
