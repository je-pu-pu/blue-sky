#include "AnimationPlayer.h"

#include "Direct3D11Matrix.h"
#include "GameMain.h"

/**
 * コンストラクタ
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
 * アニメーションを再生する
 *
 * @param name アニメーション名
 * @param force 強制的に最初から再生するフラグ
 * @param loop ループ再生フラグ
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
 * 描画用のデータを更新する
 *
 */
void AnimationPlayer::update_render_data() const
{
	if ( ! get_current_skinning_animation() )
	{
		blue_sky::BoneConstantBufferData data;

		constant_buffer_.update( & data );

		return;
	}

	blue_sky::BoneConstantBufferData data;

	for ( uint_t n = 0; n < get_current_skinning_animation()->get_bone_animation_list_size(); ++n )
	{
		const Matrix& bone_offset = get_skinning_animation_set()->get_bone_offset_matrix_by_bone_index(  n );
		const Matrix inversed_bone_offset = bone_offset.inverse();
		const Animation& bone_animation = get_current_skinning_animation()->get_bone_animation_by_bone_index( n );

		Matrix s, r, t;
		Matrix& m = data.bone_matrix[ n ];

		s.set_scaling(
			bone_animation.get_value( "sx", current_frame_ ),
			bone_animation.get_value( "sy", current_frame_ ),
			bone_animation.get_value( "sz", current_frame_ ) );

		r.set_rotation(
			bone_animation.get_value( "rx", current_frame_ ),
			bone_animation.get_value( "ry", current_frame_ ),
			bone_animation.get_value( "rz", current_frame_ ) );

		t.set_translation(
			bone_animation.get_value( "tx", current_frame_ ),
			bone_animation.get_value( "ty", current_frame_ ),
			bone_animation.get_value( "tz", current_frame_ ) );

		m = inversed_bone_offset * s * r * t * bone_offset;
	}

	constant_buffer_.update( & data );
	constant_buffer_.bind_to_vs();
}

/**
 * 描画用のデータをシェーダーにバインドする
 *
 */
void AnimationPlayer::bind_render_data() const
{
	constant_buffer_.bind_to_vs();
}
