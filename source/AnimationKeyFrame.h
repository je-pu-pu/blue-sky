#ifndef ANIMATION_KEY_FRAME_H
#define ANIMATION_KEY_FRAME_H

#include "type.h"

/**
 * アニメーションのキーフレーム
 *
 */
class AnimationKeyFrame
{
public:
	class FrameLess
	{
	public:
		bool operator () ( const AnimationKeyFrame& a, const AnimationKeyFrame& b )
		{
			return a.frame_ < b.frame_;
		}

		bool operator () ( const AnimationKeyFrame& a, float_t frame )
		{
			return a.frame_ < frame;
		}
	};

private:
	float_t frame_;			///< フレーム
	float_t value_;			///< 値

	AnimationKeyFrame();

public:
	AnimationKeyFrame( float_t frame, float_t value )
		: frame_( frame )
		, value_( value )
	{ }

	float_t& get_frame() { return frame_; }
	float_t& get_value() { return value_; }

	float_t get_frame() const { return frame_; }
	float_t get_value() const { return value_; }

}; // class AnimationKeyFrame

#endif // ANIMATION_KEY_FRAME_H
