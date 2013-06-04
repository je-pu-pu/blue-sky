#ifndef ANIMATION_PLAYER_H
#define ANIMATION_PLAYER_H

#include "Animation.h"

/**
 * アニメーション再生
 *
 */
class AnimationPlayer
{
public:
	typedef AnimationKeyFrame KeyFrame;
	typedef Channel::KeyFrameList KeyFrameList;

private:
	const Animation* animation_;	///< 現在のアニメーション
	float current_frame_;			///< 現在の再生フレーム

public:
	AnimationPlayer()
		: animation_( 0 )
		, current_frame( 0.f )
	{ }

	const Animation* get_animation() const { return animation_; }

}; // class AnimationPlayer

#endif // ANIMATION_PLAYER_H
