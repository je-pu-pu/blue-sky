#include "SoundEngine.h"
#include "SoundBuffer.h"
#include <core/sound/DirectSound/DirectSound.h>
#include <core/sound/DirectSound/DirectSoundBuffer.h>
#include <core/sound/Sound.h>
#include <core/DirectX.h>
#include <core/type.h>

#include <game/SoundFormat.h>

namespace core::sound::direct_sound
{

SoundEngine::SoundEngine( HWND hwnd )
	: direct_sound_( new DirectSound( hwnd ) )
{
	format_ = {
		.channels = direct_sound_->get_format().nChannels,
		.sampling_rate = static_cast< int >( direct_sound_->get_format().nSamplesPerSec ),
		.bit_depth = direct_sound_->get_format().wBitsPerSample
	};	
}

SoundEngine::~SoundEngine()
{

}

void SoundEngine::set_volume( float volume )
{
	LONG long_volume = static_cast< LONG >( ( volume - Sound::VOLUME_MIN ) / ( Sound::VOLUME_MAX - Sound::VOLUME_MIN ) * ( DSBVOLUME_MAX - DSBVOLUME_MIN ) + DSBVOLUME_MIN );
	DIRECT_X_FAIL_CHECK( direct_sound_->get_primary_buffer()->SetVolume( long_volume ) );
}

SoundEngine::SoundBuffer* SoundEngine::create_sound_buffer( bool is_3d_sound, bool is_streaming, size_t size, const SoundFormat& format )
{
	DSBUFFERDESC buffer_desc = { sizeof( DSBUFFERDESC ) };

	if ( is_3d_sound )
	{
		buffer_desc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRL3D;
		buffer_desc.guid3DAlgorithm = DS3DALG_DEFAULT;
	}
	else
	{
		buffer_desc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY;
	}

	if ( ! is_streaming )
	{
		buffer_desc.dwFlags |= DSBCAPS_STATIC;
	}

	WAVEFORMATEX wave_format = {
		.wFormatTag = WAVE_FORMAT_PCM,
		.nChannels = static_cast< WORD >( format.channels ),
		.nSamplesPerSec = static_cast< DWORD >( format.sampling_rate ),
		.nAvgBytesPerSec = static_cast< DWORD >( 2 * format.sampling_rate * format.channels ),
		.nBlockAlign = static_cast< WORD >( 2 * format.channels ),
		.wBitsPerSample = static_cast< WORD >( format.bit_depth ),
		.cbSize = 0,
	};

	buffer_desc.dwBufferBytes = size;
	buffer_desc.lpwfxFormat = & wave_format;

	DirectSoundBuffer* direct_sound_buffer = direct_sound_->create_sound_buffer( buffer_desc );
	direct_sound_buffer->set_3d_sound( is_3d_sound );

	return new SoundBuffer( direct_sound_buffer );
}

void SoundEngine::set_listener_position( const Vector3& p )
{
	direct_sound_->get_listener()->SetPosition( p.x(), p.y(), p.z(), DS3D_DEFERRED );
}

void SoundEngine::set_listener_velocity( const Vector3& v )
{
	direct_sound_->get_listener()->SetVelocity( v.x(), v.y(), v.z(), DS3D_DEFERRED );
}

void SoundEngine::set_listener_orientation( const Vector3& front, const Vector3& top )
{
	direct_sound_->get_listener()->SetOrientation( front.x(), front.y(), front.z(), top.x(), top.y(), top.z(), DS3D_DEFERRED );
}

void SoundEngine::commit()
{
	direct_sound_->get_listener()->CommitDeferredSettings();
}

} // namespace core::sound::direct_sound
