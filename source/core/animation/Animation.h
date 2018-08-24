#pragma once

#include "AnimationChannel.h"
#include <array>

namespace core
{

/**
 * アニメーション
 *
 */
class Animation
{
public:
	enum ChannelIndex
	{
		CHANNEL_INDEX_TX = 0,
		CHANNEL_INDEX_TY,
		CHANNEL_INDEX_TZ,
		CHANNEL_INDEX_RX,
		CHANNEL_INDEX_RY,
		CHANNEL_INDEX_RZ,
		CHANNEL_INDEX_SX,
		CHANNEL_INDEX_SY,
		CHANNEL_INDEX_SZ,
		CHANNEL_INDEX_MAX
	};

	using Channel		= AnimationChannel;
	using ChannelList	= std::array< Channel, CHANNEL_INDEX_MAX >;

	using KeyFrame		= AnimationKeyFrame;
	using KeyFrameList	= Channel::KeyFrameList;

private:
	string_t name_;
	ChannelList channel_list_;

public:
	Animation()
	{ }

	void set_name( string_t name ) { name_ = name; }
	const string_t& get_name() const { return name_; }

	bool has_channel( ChannelIndex index ) const
	{
		return index >= 0 && static_cast< uint_t >( index ) < channel_list_.size();
	}

	Channel& get_channel( uint_t index )
	{
		return channel_list_[ index ];
	}

	/**
	 * 指定したチャンネルの指定したフレームの値を取得する
	 *
	 * @param channel_index チャンネルのインデックス
	 * @param frame フレーム
	 * @param default_value チャンネルが存在しない場合に返すデフォルト値
	 * @return 値
	 */
	float_t get_value( ChannelIndex channel_index, float_t frame, float_t default_value ) const
	{
		if ( ! has_channel( channel_index ) )
		{
			return default_value;
		}

		return channel_list_[ channel_index ].get_value( frame );
	}

	/**
	 * 最適化する
	 *
	 */
	void optimize()
	{
		//
	}

	/**
	 * アニメーションの長さを計算する
	 *
	 * @return アニメーションの長さ
	 */
	float_t calculate_length()
	{
		float_t last_frame = 0.f;

		for ( auto i = channel_list_.begin(); i != channel_list_.end(); ++i )
		{
			last_frame = std::max( last_frame, i->get_last_key_frame().get_frame() );
		}

		return last_frame;
	}

}; // class Animation

} // namespace core
