#ifndef ANIMATION_PLAYER_H
#define ANIMATION_PLAYER_H

#include "SkinningAnimationSet.h"
#include "ConstantBuffer.h"

class Direct3D11Matrix;

/**
 * スキニングアニメーションの再生
 *
 */
class AnimationPlayer
{
public:
	/// @todo 整理する
	typedef blue_sky::BoneConstantBuffer			BoneConstantBuffer;

	static const int MaxBones = blue_sky::BoneConstantBufferData::MaxBones;

private:
	const SkinningAnimationSet* skinning_animation_set_;	///< スキニングアニメーションのセット
	const SkinningAnimation* current_skinning_animation_;	///< 現在のスキニングアニメーション
	float_t	speed_;											///< 再生スピード
	float_t current_frame_;									///< 現在の再生フレーム
	bool is_looping_;										///< ループ再生中フラグ

	/// @todo 移動する
	const BoneConstantBuffer constant_buffer_;				///< 定数バッファ

	Matrix get_bone_local_matrix( uint_t ) const;

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

}; // class AnimationPlayer

#endif // ANIMATION_PLAYER_H
