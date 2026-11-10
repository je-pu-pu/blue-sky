#pragma once

#include "SkinningAnimationSet.h"
#include <core/ConstantBuffers.h>
#include <core/graphics/ConstantBuffer.h>
#include <memory>

namespace core
{

/**
 * スキニングアニメーションの再生
 *
 */
class AnimationPlayer
{
public:
	using BoneConstantBufferData = core::BoneConstantBufferData;	///< ボーン行列（使われていなかった別名・定数を整理した）

private:
	const SkinningAnimationSet* skinning_animation_set_;	///< スキニングアニメーションのセット
	const SkinningAnimation* current_skinning_animation_;	///< 現在のスキニングアニメーション
	float_t	speed_;											///< 再生スピード
	float_t current_frame_;									///< 現在の再生フレーム
	bool is_looping_;										///< ループ再生中フラグ
	bool is_broken_;										///< 破壊フラグ

	/// @todo 移動する
	std::unique_ptr< const core::graphics::ConstantBuffer > constant_buffer_;	///< ボーン行列の定数バッファ（作り手が渡す。以前は自分で作り、中で Direct3D11 の get_instance() を使っていた）

	Matrix get_bone_local_matrix( uint_t ) const;

	// コピーしない（以前は代入演算子が値を返さず、使うと未定義動作だった）
	AnimationPlayer( const AnimationPlayer& ) = delete;
	AnimationPlayer& operator = ( const AnimationPlayer& ) = delete;

public:
	/// constant_buffer は BoneConstantBufferData の大きさで BoneConstantBuffer::SLOT に作ったもの
	AnimationPlayer( const SkinningAnimationSet*, std::unique_ptr< const core::graphics::ConstantBuffer > constant_buffer );
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

	void calculate_bone_matrix_recursive( BoneConstantBufferData&, uint_t, const Matrix& ) const;

	void update_render_data() const;
	void bind_render_data() const;

	const core::graphics::ConstantBuffer* get_constant_buffer() const { return constant_buffer_.get(); }

	bool is_broken() const { return is_broken_; }
	void set_broken( bool b ) { is_broken_ = b; }

}; // class AnimationPlayer

} // namespace core
