#include "Sound.h"
#include "OggVorbisFile.h"
#include <core/sound/SoundEngine.h>
#include <core/sound/SoundBuffer.h>
#include <common/math.h>
#include <common/exception.h>

namespace game
{

const Sound::T Sound::VOLUME_MIN = 0.f;
const Sound::T Sound::VOLUME_MAX = 1.f;
const Sound::T Sound::VOLUME_FADE_SPEED_DEFAULT = 0.0025f;
const Sound::T Sound::VOLUME_FADE_SPEED_FAST = 0.025f;

const Sound::T Sound::PAN_LEFT = -1.f;
const Sound::T Sound::PAN_RIGHT = 1.f;
const Sound::T Sound::PAN_CENTER = 0.f;

}

namespace core
{

Sound::Sound( SoundEngine* sound_engine )
	: sound_engine_( sound_engine )
	, is_3d_sound_( false )
	, max_volume_( VOLUME_MAX )
	, volume_fade_( 0.f )
{

}

Sound::~Sound()
{
}

bool Sound::load( const char* file_name )
{
	if ( sound_buffer_ )
	{
		COMMON_THROW_EXCEPTION;
	}

	if ( sound_file_ )
	{
		COMMON_THROW_EXCEPTION;
	}

	sound_file_.reset( new SoundFile( file_name ) );

	sound_buffer_.reset( sound_engine_->create_sound_buffer( is_3d_sound(), false, sound_file_->size(), sound_file_->format() ) );

	if ( is_3d_sound() )
	{
		sound_buffer_->init_3d_sound();
	}

	void* data = sound_buffer_->lock();

	sound_file_->read( data, sound_file_->size() );

	sound_buffer_->unlock();

	sound_sample_buffer_.resize( sound_file_->size() / sizeof( SoundSample ) );
	memcpy( & sound_sample_buffer_[ 0 ], data, sound_file_->size() );

	return true;
}

void Sound::set_3d_position( T x, T y, T z )
{
	sound_buffer_->set_3d_position( Vector3( x, y, z ) );
}

void Sound::set_3d_velocity( T x, T y, T z )
{
	sound_buffer_->set_3d_velocity( Vector3( x, y, z ) );
}

Sound::T Sound::get_volume() const
{
	return sound_buffer_->get_volume();
}

void Sound::set_volume( T v )
{
	v = ::math::clamp( v, VOLUME_MIN, get_max_volume() );

	sound_buffer_->set_volume( v );
}

Sound::T Sound::get_speed() const
{
	return sound_buffer_->get_speed();
}

void Sound::set_speed( Sound::T s )
{
	sound_buffer_->set_speed( s );
}

bool Sound::play( bool loop, bool force )
{
	volume_fade_ = 0.f;

	if ( ! force && is_playing() )
	{
		return true;
	}

	sound_buffer_->play( loop );

	return true;
}

bool Sound::is_playing() const
{
	return sound_buffer_->is_playing();
}

bool Sound::stop()
{
	sound_buffer_->stop();

	return true;
}

void Sound::fade_in( T speed )
{
	volume_fade_ = speed;
}

void Sound::fade_out( T speed )
{
	volume_fade_ = -speed;
}

bool Sound::is_fade_full_in() const
{
	return get_volume() == get_max_volume();
}

bool Sound::is_fade_full_out() const
{
	return get_volume() == VOLUME_MIN;
}

float Sound::get_current_position() const
{
	return static_cast< float >( sound_buffer_->get_current_position() ) / static_cast< float >( sound_file_->size_per_sec() );
}

void Sound::update()
{
	if ( volume_fade_ )
	{
		set_volume( get_volume() + volume_fade_ );
	
		if ( get_volume() < VOLUME_MAX * 0.5f )
		{
			volume_fade_ = 0.f;
		}
	}
}

} // namespace core
