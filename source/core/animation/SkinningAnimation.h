#pragma once

#include "Animation.h"

namespace core
{

/**
 * スキニングアニメーション
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
	AnimationList	bone_animation_list_;					///< ボーン毎のアニメーションの一覧
	float_t			length_;								///< アニメーションの長さ ( フレーム )

public:
	SkinningAnimation( uint_t size )
		: bone_animation_list_( size )
		, length_( 0.f )
	{

	}

	/**
	 * 指定したボーンインデックスに対応するアニメーションを取得する
	 *
	 * @param bone_index ボーンインデックス
	 * @return アニメーション
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
	 * ボーン毎のアニメーションのサイズを取得する
	 *
	 * @return ボーン毎のアニメーションの一覧のサイズ
	 */
	uint_t get_bone_animation_list_size() const
	{
		return bone_animation_list_.size();
	}

	/**
	 * ボーン毎のアニメーションの一覧をリサイズする
	 *
	 * @param size ボーン毎のアニメーションの一覧のサイズ
	 */
	void resize_bone_animation_list( uint_t size )
	{
		bone_animation_list_.resize( size );
	}

	/**
	 * 最適化する
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
	 * アニメーションの長さを計算する
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
	 * アニメーションの長さを取得する
	 *
	 */
	float_t get_length() const
	{
		return length_;
	}

}; // class SkinningAnimation

} // namespace core
