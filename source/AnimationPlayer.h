#ifndef ANIMATION_PLAYER_H
#define ANIMATION_PLAYER_H

#include "Animation.h"

/**
 * �A�j���[�V�����Đ�
 *
 */
class AnimationPlayer
{
public:
	typedef AnimationKeyFrame KeyFrame;
	typedef Channel::KeyFrameList KeyFrameList;

private:
	const Animation* animation_;	///< ���݂̃A�j���[�V����
	float current_frame_;			///< ���݂̍Đ��t���[��

public:
	AnimationPlayer()
		: animation_( 0 )
		, current_frame( 0.f )
	{ }

	const Animation* get_animation() const { return animation_; }

}; // class AnimationPlayer

#endif // ANIMATION_PLAYER_H
