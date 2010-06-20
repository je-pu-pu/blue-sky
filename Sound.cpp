#include "Sound.h"
#include "DirectSoundBuffer.h"
#include <common/exception.h>

namespace game
{

const Sound::T Sound::VOLUME_MIN = 0.f;
const Sound::T Sound::VOLUME_MAX = 1.f;

const Sound::T Sound::PAN_LEFT = -1.f;
const Sound::T Sound::PAN_RIGHT = 1.f;
const Sound::T Sound::PAN_CENTER = 0.f;

}

namespace blue_sky
{

Sound::Sound()
	: direct_sound_buffer_( 0 )
{

}

Sound::~Sound()
{
	delete direct_sound_buffer_;
}

bool Sound::load( const char* file_name )
{
	// 
	COMMON_THROW_EXCEPTION;
}

Sound::T Sound::get_speed() const
{
	return direct_sound_buffer_->getSpeed();
}

void Sound::set_speed( Sound::T s )
{
	return direct_sound_buffer_->setSpeed( s );
}

bool Sound::play( bool loop )
{
	direct_sound_buffer_->play( loop );

	return true;
}

} // namespace blue_sky
