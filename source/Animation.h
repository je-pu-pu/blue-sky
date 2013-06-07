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
	string_t name_;
	ChannelMap channel_map_;

public:
	Animation()
	{ }

	void set_name( string_t name ) { name_ = name; }
	const string_t& get_name() const { return name_; }

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

	/**
	 * アニメーションの長さを計算する
	 *
	 * @return アニメーションの長さ
	 */
	float_t calculate_length()
	{
		float_t last_frame = 0.f;

		for ( auto i = channel_map_.begin(); i != channel_map_.end(); ++i )
		{
			last_frame = std::max( last_frame, i->second.get_last_key_frame().get_frame() );
		}

		return last_frame;
	}

}; // class Animation

#endif // ANIMATION_H
