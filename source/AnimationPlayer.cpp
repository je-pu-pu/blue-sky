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

/**
 * アニメーションを更新する
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
 * 描画用のデータを更新する
 *
 * @todo 必要最低限のボーン行列だけ更新するようにする
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

	calculate_bone_matrix_recursive( data, 0, Matrix() );

	for ( uint_t n = 0; n < get_skinning_animation_set()->get_bone_count(); ++n )
	{
		data.bone_matrix[ n ] = data.bone_matrix[ n ].transpose();
	}

	constant_buffer_.update( & data );
}

/**
 * ボーンマトリックスを再帰的に計算する
 *
 * @param data ボーンマトリックス
 * @param bone_index ボーンのインデックス
 * @param parent_bone_matrix 親ボーン行列
 */
void AnimationPlayer::calculate_bone_matrix_recursive( BoneConstantBuffer::Data& data, uint_t bone_index, const Matrix& parent_bone_matrix ) const
{
	const Animation& bone_animation = get_current_skinning_animation()->get_bone_animation_by_bone_index( bone_index );
	const Matrix& bone_offset_matrix = get_skinning_animation_set()->get_bone_offset_matrix_by_bone_index( bone_index );

	uint_t parent_bone_index = get_skinning_animation_set()->get_parent_bone_index( bone_index );
	const Matrix& parent_bone_offset_matrix = bone_index == 0 ? Matrix() : get_skinning_animation_set()->get_bone_offset_matrix_by_bone_index( parent_bone_index );

	Matrix s, r, t;

	s.set_scaling(
		bone_animation.get_value( "sx", current_frame_, 1.f ),
		bone_animation.get_value( "sy", current_frame_, 1.f ),
		bone_animation.get_value( "sz", current_frame_, 1.f ) );

	r.set_rotation_roll_pitch_yaw(
		bone_animation.get_value( "rx", current_frame_, 0.f ),
		bone_animation.get_value( "ry", current_frame_, 0.f ),
		bone_animation.get_value( "rz", current_frame_, 0.f ) );

	t.set_translation(
		bone_animation.get_value( "tx", current_frame_, 0.f ),
		bone_animation.get_value( "ty", current_frame_, 0.f ),
		bone_animation.get_value( "tz", current_frame_, 0.f ) );

	Matrix rx, ry, rz;

	rx.set_rotation_x( bone_animation.get_value( "rx", current_frame_, 0.f ) );
	ry.set_rotation_y( bone_animation.get_value( "ry", current_frame_, 0.f ) );
	rz.set_rotation_z( bone_animation.get_value( "rz", current_frame_, 0.f ) );

	data.bone_matrix[ bone_index ] = bone_offset_matrix.inverse() * s * rx * ry * rz * t * parent_bone_offset_matrix * parent_bone_matrix;

	for ( uint_t n = 0; n < get_skinning_animation_set()->get_child_bone_count( bone_index ); ++n )
	{
		calculate_bone_matrix_recursive( data, get_skinning_animation_set()->get_child_bone_index( bone_index, n ), data.bone_matrix[ bone_index ] );
	}
}

/**
 * 描画用のデータをシェーダーにバインドする
 *
 */
void AnimationPlayer::bind_render_data() const
{
	constant_buffer_.bind_to_vs();
}
