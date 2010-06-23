#include "StreamingSound.h"
#include "DirectSound.h"
#include "DirectSoundBuffer.h"
#include "DirectX.h"
#include "WaveFile.h"
#include "OggVorbisFile.h"

// #include <common/math.h>
#include <common/exception.h>

namespace blue_sky
{

StreamingSound::StreamingSound( const DirectSound* direct_sound )
	: Sound( direct_sound )
	, is_first_half_playing_( true )
{

}

StreamingSound::~StreamingSound()
{
	
}

bool StreamingSound::load( const char* file_name )
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
	buffer_desc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY;
	buffer_desc.dwBufferBytes = std::min( sound_file_->size(), get_buffer_size() );
	buffer_desc.lpwfxFormat = & sound_file_->format();

	direct_sound_buffer_ = direct_sound_->create_sound_buffer( buffer_desc );

	void* data = 0;
	DWORD size = 0;

	direct_sound_buffer_->get_direct_sound_buffer()->Lock( 0, 0, & data, & size, 0, 0, DSBLOCK_ENTIREBUFFER );
	
	sound_file_->read( data, buffer_desc.dwBufferBytes );
	
	direct_sound_buffer_->get_direct_sound_buffer()->Unlock( data, size, 0, 0 );

	return true;
}

bool StreamingSound::play( bool loop )
{
	direct_sound_buffer_->play( true );

	return true;
}

void StreamingSound::update()
{
	DWORD play_pos = 0;
	DIRECT_X_FAIL_CHECK( direct_sound_buffer_->get_direct_sound_buffer()->GetCurrentPosition( & play_pos, 0 ) );

	bool swap = false;
	DWORD lock_offset = 0;
	DWORD lock_size = 0;
	
	if ( is_first_half_playing_ && play_pos >= direct_sound_buffer_->get_caps().dwBufferBytes / 2 )
	{
		swap = true;
		lock_size = direct_sound_buffer_->get_caps().dwBufferBytes / 2;
	}
	else if ( ! is_first_half_playing_ && play_pos < direct_sound_buffer_->get_caps().dwBufferBytes / 2 ) 
	{
		swap = true;
		lock_offset = direct_sound_buffer_->get_caps().dwBufferBytes / 2;
		lock_size = direct_sound_buffer_->get_caps().dwBufferBytes - lock_offset;
	}

	if ( swap )
	{
		is_first_half_playing_ = ! is_first_half_playing_;

		void* data = 0;
		DWORD size = 0;

		DIRECT_X_FAIL_CHECK( direct_sound_buffer_->get_direct_sound_buffer()->Lock( lock_offset, lock_size, & data, & size, 0, 0, 0 ) );

		int x = sound_file_->read( data, size );

		DIRECT_X_FAIL_CHECK( direct_sound_buffer_->get_direct_sound_buffer()->Unlock( data, size, 0, 0 ) );
	}
}

} // namespace blue_sky
