#include "SoundBuffer.h"
#include "DirectSoundBuffer.h"
#include <core/sound/Sound.h>
#include <core/DirectX.h>

#include <iostream>

namespace core::sound::direct_sound
{

SoundBuffer::SoundBuffer( DirectSoundBuffer* direct_sound_buffer )
	: direct_sound_buffer_( direct_sound_buffer )
{
	
}

void SoundBuffer::init_3d_sound()
{
	DIRECT_X_FAIL_CHECK( direct_sound_buffer_->get_direct_sound_3d_buffer()->SetMinDistance( 2.f, DS3D_DEFERRED ) );
	DIRECT_X_FAIL_CHECK( direct_sound_buffer_->get_direct_sound_3d_buffer()->SetMaxDistance( 50.f, DS3D_DEFERRED ) );
	DIRECT_X_FAIL_CHECK( direct_sound_buffer_->get_direct_sound_3d_buffer()->SetMode( DS3DMODE_NORMAL, DS3D_DEFERRED ) );
}

void* SoundBuffer::lock( size_t offset = 0, size_t size = 0 )
{
	data_ = 0;
	locked_size_ = 0;

	DWORD flags = 0;

	if ( offset == 0 && size == 0 )
	{
		flags |= DSBLOCK_ENTIREBUFFER;
	}

	std::cout << "o:" << offset << ",s:" << size << std::endl;

	DIRECT_X_FAIL_CHECK( direct_sound_buffer_->get_direct_sound_buffer()->Lock( offset, size, & data_, & locked_size_, 0, 0, flags ) );

	return data_;
}

void SoundBuffer::unlock()
{
	DIRECT_X_FAIL_CHECK( direct_sound_buffer_->get_direct_sound_buffer()->Unlock( data_, locked_size_, 0, 0 ) );
}

void SoundBuffer::set_3d_position( const Vector3& v )
{
	if ( ! direct_sound_buffer_->get_direct_sound_3d_buffer() )
	{
		return;
	}

	DIRECT_X_FAIL_CHECK( direct_sound_buffer_->get_direct_sound_3d_buffer()->SetPosition( v.x(), v.y(), v.z(), DS3D_DEFERRED ) );
}

void SoundBuffer::set_3d_velocity( const Vector3& v )
{
	if ( ! direct_sound_buffer_->get_direct_sound_3d_buffer() )
	{
		return;
	}

	DIRECT_X_FAIL_CHECK( direct_sound_buffer_->get_direct_sound_3d_buffer()->SetVelocity( v.x(), v.y(), v.z(), DS3D_DEFERRED ) );
}

float SoundBuffer::get_volume() const
{
	LONG volume = 0;
	DIRECT_X_FAIL_CHECK( direct_sound_buffer_->get_direct_sound_buffer()->GetVolume( & volume ) );

	return static_cast< float >( volume - DSBVOLUME_MIN ) / ( DSBVOLUME_MAX - DSBVOLUME_MIN ) * ( Sound::VOLUME_MAX - Sound::VOLUME_MIN ) - Sound::VOLUME_MIN;
}

void SoundBuffer::set_volume( float v )
{
	LONG volume = static_cast< long >( ( v  - Sound::VOLUME_MIN ) / ( Sound::VOLUME_MAX - Sound::VOLUME_MIN ) * ( DSBVOLUME_MAX - DSBVOLUME_MIN ) + DSBVOLUME_MIN );
	DIRECT_X_FAIL_CHECK( direct_sound_buffer_->get_direct_sound_buffer()->SetVolume( volume ) );
}

float SoundBuffer::get_speed() const
{
	return direct_sound_buffer_->getSpeed();
}

void SoundBuffer::set_speed( float s )
{
	direct_sound_buffer_->setSpeed( s );
}

void SoundBuffer::play( bool loop )
{
	direct_sound_buffer_->play( loop );
}

bool SoundBuffer::is_playing() const
{
	DWORD status = 0;
	DIRECT_X_FAIL_CHECK( direct_sound_buffer_->get_direct_sound_buffer()->GetStatus( & status ) );

	return status & DSBSTATUS_PLAYING;
}

void SoundBuffer::stop()
{
	direct_sound_buffer_->get_direct_sound_buffer()->Stop();
}

size_t SoundBuffer::get_size() const
{
	return direct_sound_buffer_->get_caps().dwBufferBytes;
}

size_t SoundBuffer::get_current_position() const
{
	DWORD play_pos = 0;
	DIRECT_X_FAIL_CHECK( direct_sound_buffer_->get_direct_sound_buffer()->GetCurrentPosition( & play_pos, 0 ) );

	return play_pos;
}

} // namespace core::sound::direct_sound
