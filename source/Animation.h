#ifndef ANIMATION_H
#define ANIMATION_H

#include "AnimationChannel.h"
#include <map>

/**
 * �A�j���[�V����
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
	 * �w�肵���`�����l���̎w�肵���t���[���̒l���擾����
	 *
	 * @param name �`�����l����
	 * @param frame �t���[��
	 * @param default_value �`�����l�������݂��Ȃ��ꍇ�ɕԂ��f�t�H���g�l
	 * @return �l
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
	 * �œK������
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
	 * �A�j���[�V�����̒������v�Z����
	 *
	 * @return �A�j���[�V�����̒���
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
