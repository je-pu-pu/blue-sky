#include "StreamingSound.h"
#include "DirectSound.h"
#include "DirectSoundBuffer.h"
#include "WaveFile.h"

#include <common/exception.h>

namespace blue_sky
{

StreamingSound::StreamingSound( const DirectSound* direct_sound )
	: Sound( direct_sound )
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

	WaveFile wave_file( file_name );

	DSBUFFERDESC buffer_desc = { sizeof( DSBUFFERDESC ) };
	buffer_desc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY;
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

} // namespace blue_sky
