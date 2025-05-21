#include "StreamingSound.h"
#include "SoundEngine.h"
#include "SoundBuffer.h"
#include "OggVorbisFile.h"

#include <common/exception.h>

#include <algorithm>

namespace core
{

StreamingSound::StreamingSound( SoundEngine* sound_engine )
	: Sound( sound_engine )
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
	if ( sound_buffer_ )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( "sound_buffer_ is not null." );
	}

	if ( sound_file_ )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( "sound_file_ is not null." );
	}

	sound_file_.reset( new SoundFile( file_name ) );

	sound_buffer_.reset( sound_engine_->create_sound_buffer( is_3d_sound(), true, std::min( sound_file_->size(), get_buffer_size() ), sound_file_->format() ) );

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

	sound_buffer_->play( true );

	return true;
}

void StreamingSound::update()
{
	Sound::update();

	// 再生終了チェック
	if ( ! is_loop_ )
	{
		if ( get_current_position() >= static_cast< float >( sound_file_->size() ) / static_cast< float >( sound_file_->size_per_sec() ) )
		{
			stop();
		}
	}

	auto pos = sound_buffer_->get_current_position();

	// バッファ書き込みチェック
	if ( is_first_half_playing_ && pos >= sound_buffer_->get_size() / 2 )
	{
		stream_half( false );
		is_first_half_playing_ = ! is_first_half_playing_;
	}
	else if ( ! is_first_half_playing_ && pos < sound_buffer_->get_size() / 2 ) 
	{
		stream_half( true );
		is_first_half_playing_ = ! is_first_half_playing_;
		current_position_offset_ += static_cast< float >( sound_buffer_->get_size() ) / static_cast< float >( sound_file_->size_per_sec() );
	}
}

void StreamingSound::stream_all()
{
	void* data = sound_buffer_->lock();
	
	sound_file_->read( data, sound_buffer_->get_size() );
	
	sound_buffer_->unlock();

	memcpy( & sound_sample_buffer_[ 0 ], data, sound_buffer_->get_size() );
}

void StreamingSound::stream_half( bool first_half )
{
	DWORD lock_offset = 0;
	DWORD lock_size = 0;

	if ( first_half )
	{
		lock_offset = sound_buffer_->get_size() / 2;
		lock_size = sound_buffer_->get_size() - lock_offset;
	}
	else
	{
		lock_size = sound_buffer_->get_size() / 2;
	}

	void* data = sound_buffer_->lock( lock_offset, lock_size );

	sound_file_->read( data, lock_size, is_loop_ );

	sound_buffer_->unlock();

	memcpy( & sound_sample_buffer_[ lock_offset / sizeof( SoundSample ) ], data, lock_size );
}

float StreamingSound::get_current_position() const
{
	auto play_pos = sound_buffer_->get_current_position();

	return current_position_offset_ + static_cast< float >( play_pos ) / static_cast< float >( sound_file_->size_per_sec() );
}

float StreamingSound::get_current_peak_level() const
{
	auto play_pos = sound_buffer_->get_current_position();

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

} // namespace core
