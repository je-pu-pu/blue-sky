#pragma once

#include "SkinningAnimationSet.h"
#include <blue_sky/ShaderResources.h>

/**
 * スキニングアニメーションの再生
 *
 */
class AnimationPlayer
{
public:
	/// @todo 整理する
	typedef blue_sky::BoneConstantBuffer			BoneConstantBuffer;

	static const int MaxBones = blue_sky::BoneShaderResourceData::MaxBones;

private:
	const SkinningAnimationSet* skinning_animation_set_;	///< スキニングアニメーションのセット
	const SkinningAnimation* current_skinning_animation_;	///< 現在のスキニングアニメーション
	float_t	speed_;											///< 再生スピード
	float_t current_frame_;									///< 現在の再生フレーム
	bool is_looping_;										///< ループ再生中フラグ
	bool is_broken_;										///< 破壊フラグ

	/// @todo 移動する
	const BoneConstantBuffer constant_buffer_;				///< 定数バッファ

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

	void calculate_bone_matrix_recursive( BoneConstantBuffer::Data&, uint_t, const Matrix& ) const;

	void update_render_data() const;
	void bind_render_data() const;

	const BoneConstantBuffer* get_constant_buffer() const { return & constant_buffer_; }

	bool is_broken() const { return is_broken_; }
	void set_broken( bool b ) { is_broken_ = b; }

}; // class AnimationPlayer
