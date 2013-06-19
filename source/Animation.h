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
	typedef std::map< string_t, Channel > ChannelMap;

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

	bool has_channel( const char_t* name ) const
	{
		return channel_map_.find( name ) != channel_map_.end();
	}

	Channel& get_channel( const char_t* name )
	{
		return channel_map_[ name ];
	}

	/**
	 * 指定したチャンネルの指定したフレームの値を取得する
	 *
	 * @param name チャンネル名
	 * @param frame フレーム
	 * @param default_value チャンネルが存在しない場合に返すデフォルト値
	 * @return 値
	 */
	float_t get_value( const char_t* name, float_t frame, float_t default_value ) const
	{
		ChannelMap::const_iterator i = channel_map_.find( name );

		if ( i == channel_map_.end() )
		{
			return default_value;
		}

		return i->second.get_value( frame );
	}

	/**
	 * 最適化する
	 *
	 */
	void optimize()
	{
		for ( auto i = channel_map_.begin(); i != channel_map_.end(); )
		{
			if ( i->second.empty() )
			{
				i = channel_map_.erase( i );
			}
			else
			{
				++i;
			}
		}
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
