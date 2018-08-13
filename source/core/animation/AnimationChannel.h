#pragma once

#include "AnimationKeyFrame.h"
#include <vector>
#include <algorithm>

namespace core
{

/**
 * アニメーションのチャンネル
 *
 */
class AnimationChannel
{
public:
	typedef AnimationKeyFrame KeyFrame;
	typedef std::vector< KeyFrame > KeyFrameList;

private:
	KeyFrameList key_frame_list_;

public:
	AnimationChannel()
	{ }

	void add_key_frame( const KeyFrame& key_frame )
	{
		key_frame_list_.push_back( key_frame );
	}

	void sort()
	{
		std::sort( key_frame_list_.begin(), key_frame_list_.end(), KeyFrame::FrameLess() );
	}

	bool empty() const
	{
		return key_frame_list_.empty();
	}


	float_t get_value( float_t frame ) const
	{
		KeyFrameList::const_iterator next = std::lower_bound( key_frame_list_.begin(), key_frame_list_.end(), frame, KeyFrame::FrameLess() );

		if ( next == key_frame_list_.begin() )
		{
			return key_frame_list_.front().get_value();
		}

		if ( next == key_frame_list_.end() )
		{
			return key_frame_list_.back().get_value();
		}

		KeyFrameList::const_iterator current = next - 1;

		float_t ratio = ( frame - current->get_frame() ) / ( next->get_frame() - current->get_frame() );

		return current->get_value() * ( 1.f - ratio ) + next->get_value() * ratio;
	}

	KeyFrameList& get_key_frame_list() { return key_frame_list_; }
	KeyFrame& get_last_key_frame() { return key_frame_list_.back(); }

}; // class AnimationChannel

} // namespace core
