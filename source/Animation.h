#ifndef ANIMATION_H
#define ANIMATION_H

#include "AnimationChannel.h"
#include <map>

/**
 * アニメーション
 *
 */
class Animation
{
public:
	typedef AnimationChannel Channel;
	typedef std::map< const string_t, Channel > ChannelMap;

	typedef AnimationKeyFrame KeyFrame;
	typedef Channel::KeyFrameList KeyFrameList;

private:
	ChannelMap channel_map_;

public:
	Animation()
	{ }

	Channel& get_channel( const char_t* name )
	{
		return channel_map_[ name ];
	}

	float_t get_value( const char_t* name, float_t frame ) const
	{
		ChannelMap::const_iterator i = channel_map_.find( name );

		if ( i == channel_map_.end() )
		{
			return 0.f;
		}

		return i->second.get_value( frame );
	}

}; // class Animation

#endif // ANIMATION_H
