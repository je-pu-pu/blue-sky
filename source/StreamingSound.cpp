#include "StreamingSound.h"
#include "DirectSound.h"
#include "DirectSoundBuffer.h"
#include "DirectX.h"
// #include "WaveFile.h"
#include "OggVorbisFile.h"

// #include <common/math.h>
#include <common/exception.h>

#include "memory.h"

namespace blue_sky
{

StreamingSound::StreamingSound( const DirectSound* direct_sound )
	: Sound( direct_sound )
	, is_loop_( false )
	, is_first_half_playing_( true )
	, current_position_offset_( 0.f )
{

}

StreamingSound::~StreamingSound()
{
	
}

bool StreamingSound::load( const char* file_name )
{
	if ( direct_sound_buffer_ )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( "direct_sound_buffer_ is not null." );
	}

	if ( sound_file_ )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( "sound_file_ is not null." );
	}

	sound_file_ = new SoundFile( file_name );

	DSBUFFERDESC buffer_desc = { sizeof( DSBUFFERDESC ) };

	if ( is_3d_sound() )
	{
		buffer_desc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRL3D;
		buffer_desc.guid3DAlgorithm = DS3DALG_DEFAULT;
	}
	else
	{
		buffer_desc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY;
	}
	buffer_desc.dwBufferBytes = std::min( sound_file_->size(), get_buffer_size() );
	buffer_desc.lpwfxFormat = & sound_file_->format();
	buffer_desc.guid3DAlgorithm = DS3DALG_DEFAULT;

	direct_sound_buffer_ = direct_sound_->create_sound_buffer( buffer_desc );
	direct_sound_buffer_->set_3d_sound( is_3d_sound() );

	sound_sample_buffer_.resize( get_buffer_size() / sizeof( SoundSample ) );

	stream_all();

	return true;
}

bool StreamingSound::play( bool loop, bool force )
{
	volume_fade_ = 0.f;

	if ( ! force && is_playing() )
	{
		return true;
	}

	if ( get_current_position() > 0.f )
	{
		sound_file_->seek( 0 );
		stream_all();
	}

	is_loop_ = loop;
	is_first_half_playing_ = true;
	current_position_offset_ = 0.f;

	direct_sound_buffer_->play( true );

	return true;
}

void StreamingSound::update()
{
	Sound::update();

	DWORD play_pos = 0;
	DIRECT_X_FAIL_CHECK( direct_sound_buffer_->get_direct_sound_buffer()->GetCurrentPosition( & play_pos, 0 ) );

	// 再生終了チェック
	if ( ! is_loop_ )
	{
		if ( get_current_position() >= static_cast< float >( sound_file_->size() ) / static_cast< float >( sound_file_->size_per_sec() ) )
		{
			stop();
		}
	}

	// バッファ書き込みチェック
	if ( is_first_half_playing_ && play_pos >= direct_sound_buffer_->get_caps().dwBufferBytes / 2 )
	{
		stream_half( false );
		is_first_half_playing_ = ! is_first_half_playing_;
	}
	else if ( ! is_first_half_playing_ && play_pos < direct_sound_buffer_->get_caps().dwBufferBytes / 2 ) 
	{
		stream_half( true );
		is_first_half_playing_ = ! is_first_half_playing_;
		current_position_offset_ += static_cast< float >( direct_sound_buffer_->get_caps().dwBufferBytes ) / static_cast< float >( sound_file_->size_per_sec() );
	}
}

void StreamingSound::stream_all()
{
	void* data = 0;
	DWORD size = 0;

	direct_sound_buffer_->get_direct_sound_buffer()->Lock( 0, 0, & data, & size, 0, 0, DSBLOCK_ENTIREBUFFER );
	
	sound_file_->read( data, direct_sound_buffer_->get_caps().dwBufferBytes );
	
	direct_sound_buffer_->get_direct_sound_buffer()->Unlock( data, size, 0, 0 );

	memcpy( & sound_sample_buffer_[ 0 ], data, size );
}

void StreamingSound::stream_half( bool first_half )
{
	DWORD lock_offset = 0;
	DWORD lock_size = 0;

	if ( first_half )
	{
		lock_offset = direct_sound_buffer_->get_caps().dwBufferBytes / 2;
		lock_size = direct_sound_buffer_->get_caps().dwBufferBytes - lock_offset;
	}
	else
	{
		lock_size = direct_sound_buffer_->get_caps().dwBufferBytes / 2;
	}

	void* data = 0;
	DWORD size = 0;

	DIRECT_X_FAIL_CHECK( direct_sound_buffer_->get_direct_sound_buffer()->Lock( lock_offset, lock_size, & data, & size, 0, 0, 0 ) );

	int x = sound_file_->read( data, size, is_loop_ );

	DIRECT_X_FAIL_CHECK( direct_sound_buffer_->get_direct_sound_buffer()->Unlock( data, size, 0, 0 ) );

	memcpy( & sound_sample_buffer_[ lock_offset / sizeof( SoundSample ) ], data, size );
}

float StreamingSound::get_current_position() const
{
	DWORD play_pos = 0;
	direct_sound_buffer_->get_direct_sound_buffer()->GetCurrentPosition( & play_pos, 0 );

	return current_position_offset_ + static_cast< float >( play_pos ) / static_cast< float >( sound_file_->size_per_sec() );
}

float StreamingSound::get_current_peak_level() const
{
	DWORD play_pos = 0;
	direct_sound_buffer_->get_direct_sound_buffer()->GetCurrentPosition( & play_pos, 0 );

	const float sample_sec = 1.f / 20.f;
	const int sample_count = static_cast< int >( sound_file_->size_per_sec() * sample_sec ) / sizeof( SoundSample );
	float result = 0.f;

	for ( int n = 0; n < sample_count; n++ )
	{
		DWORD sample_index = ( play_pos / sizeof( SoundSample ) + n ) % sound_sample_buffer_.size();
		result = std::max( result, sound_sample_buffer_[ sample_index ] / static_cast< float >( USHRT_MAX ) );
	}

	return result;
}

} // namespace blue_sky
