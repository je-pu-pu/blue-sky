#include "AnimationPlayer.h"

#include "Direct3D11Matrix.h"
#include "GameMain.h"

#include "memory.h"

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
 * 現在再生中のアニメーションを停止する
 *
 */
void AnimationPlayer::stop()
{
	current_skinning_animation_ = nullptr;
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
 * ボーンマトリックスを再帰的に計算する
 *
 * @param data ボーンマトリックス
 * @param bone_index ボーンのインデックス
 * @param parent_bone_matrix 親ボーン行列
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
 * 指定したインデックスのボーンのローカルマトリックスを取得する
 *
 * @param bone_index ボーンのインデックス
 * @return ボーンのローカルマトリックス
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
 * 描画用のデータをシェーダーにバインドする
 *
 */
void AnimationPlayer::bind_render_data() const
{
	constant_buffer_.bind_to_vs();
}
