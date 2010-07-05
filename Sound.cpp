#include "Sound.h"
#include "DirectSound.h"
#include "DirectSoundBuffer.h"
#include "DirectX.h"
#include "WaveFile.h"
#include "OggVorbisFile.h"

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
	, sound_file_( 0 )
{

}

Sound::~Sound()
{
	delete direct_sound_buffer_;
	delete sound_file_;
}

bool Sound::load( const char* file_name )
{
	if ( direct_sound_buffer_ )
	{
		COMMON_THROW_EXCEPTION;
	}

	if ( sound_file_ )
	{
		COMMON_THROW_EXCEPTION;
	}

	sound_file_ = new SoundFile( file_name );

	DSBUFFERDESC buffer_desc = { sizeof( DSBUFFERDESC ) };
	buffer_desc.dwFlags = DSBCAPS_STATIC | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY;
	buffer_desc.dwBufferBytes = sound_file_->size();
	buffer_desc.lpwfxFormat = & sound_file_->format();

	direct_sound_buffer_ = direct_sound_->create_sound_buffer( buffer_desc );

	void* data = 0;
	DWORD size = 0;

	direct_sound_buffer_->get_direct_sound_buffer()->Lock( 0, 0, & data, & size, 0, 0, DSBLOCK_ENTIREBUFFER );
	
	sound_file_->read( data, sound_file_->size() );
	
	direct_sound_buffer_->get_direct_sound_buffer()->Unlock( data, size, 0, 0 );

	return true;
}

Sound::T Sound::get_volume()
{
	LONG volume = 0;
	DIRECT_X_FAIL_CHECK( direct_sound_buffer_->get_direct_sound_buffer()->GetVolume( & volume ) );

	return static_cast< T >( volume - DSBVOLUME_MIN ) / ( DSBVOLUME_MAX - DSBVOLUME_MIN ) * ( VOLUME_MAX - VOLUME_MIN ) - VOLUME_MIN;
}

void Sound::set_volume( T v )
{
	LONG volume = static_cast< long >( ( v  - VOLUME_MIN ) / ( VOLUME_MAX - VOLUME_MIN ) * ( DSBVOLUME_MAX - DSBVOLUME_MIN ) + DSBVOLUME_MIN );
	DIRECT_X_FAIL_CHECK( direct_sound_buffer_->get_direct_sound_buffer()->SetVolume( volume ) );
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