#include "Sound.h"
#include "DirectSound.h"
#include "DirectSoundBuffer.h"
#include "WaveFile.h"

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

Sound::Sound( const DirectSound* direct_sound )
	: direct_sound_( direct_sound )
	, direct_sound_buffer_( 0 )
{

}

Sound::~Sound()
{
	delete direct_sound_buffer_;
}

bool Sound::load( const char* file_name )
{
	if ( direct_sound_buffer_ )
	{
		COMMON_THROW_EXCEPTION;
	}

	WaveFile wave_file( file_name );

	DSBUFFERDESC buffer_desc = { sizeof( DSBUFFERDESC ) };
	buffer_desc.dwFlags = DSBCAPS_STATIC | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY;
	buffer_desc.dwBufferBytes = wave_file.size();
	buffer_desc.lpwfxFormat = & wave_file.format();

	direct_sound_buffer_ = direct_sound_->create_sound_buffer( buffer_desc );

	void* data = 0;
	DWORD size = 0;

	direct_sound_buffer_->get_direct_sound_buffer()->Lock( 0, 0, & data, & size, 0, 0, DSBLOCK_ENTIREBUFFER );
	
	memcpy( data, wave_file.data(), wave_file.size() );
	
	direct_sound_buffer_->get_direct_sound_buffer()->Unlock( data, size, 0, 0 );

	return true;
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

bool Sound::stop()
{
	direct_sound_buffer_->get_direct_sound_buffer()->Stop();

	return true;
}

} // namespace blue_sky
