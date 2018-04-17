#include "DirectSound.h"
#include "DirectSoundBuffer.h"
#include "DirectX.h"

#include <common/exception.h>
#include <string>

#pragma comment( lib, "dsound.lib" )

DirectSound::DirectSound( HWND hwnd )
	: direct_sound_( 0 )
	, primary_buffer_( 0 )
	, listener_( 0 )
{
	DIRECT_X_FAIL_CHECK( DirectSoundCreate8( NULL, & direct_sound_, 0 ) );
	DIRECT_X_FAIL_CHECK( direct_sound_->SetCooperativeLevel( hwnd, DSSCL_PRIORITY ) );

	caps_.dwSize = sizeof( DSCAPS );
	DIRECT_X_FAIL_CHECK( direct_sound_->GetCaps( & caps_ ) );

	// Create Primary Buffer
	DSBUFFERDESC buffer_desc = { sizeof( DSBUFFERDESC ) };
	buffer_desc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;

	DIRECT_X_FAIL_CHECK( direct_sound_->CreateSoundBuffer( & buffer_desc, & primary_buffer_, 0 ) );


	// 3D 
	DIRECT_X_FAIL_CHECK( primary_buffer_->QueryInterface( IID_IDirectSound3DListener8, reinterpret_cast< void** >( & listener_ ) ) );
	DIRECT_X_FAIL_CHECK( listener_->SetDopplerFactor( DS3D_DEFAULTDOPPLERFACTOR , DS3D_DEFERRED ) );
	DIRECT_X_FAIL_CHECK( listener_->SetRolloffFactor( 0.25f, DS3D_DEFERRED ) );

	/*
	PCMWAVEFORMAT format = { 0 };
	format.wf.wFormatTag = WAVE_FORMAT_PCM;
	format.wf.nChannels = 2;
	format.wf.nSamplesPerSec = 22050;
	format.wf.nBlockAlign = 4;
	format.wf.nAvgBytesPerSec = format.wf.nSamplesPerSec * format.wf.nBlockAlign;
	format.wBitsPerSample = 16;

	if ( FAILED( primary_buffer_->SetFormat( & format ) ) )
	{
		COMMON_THROW_EXCEPTION;
	}
	*/
}

DirectSound::~DirectSound()
{
	direct_sound_->Release();
}

DirectSoundBuffer* DirectSound::create_sound_buffer( const DSBUFFERDESC& buffer_desc ) const
{
	LPDIRECTSOUNDBUFFER buffer = 0;

	DIRECT_X_FAIL_CHECK( direct_sound_->CreateSoundBuffer( & buffer_desc, & buffer,  0 ) );

	LPDIRECTSOUNDBUFFER8 buffer_8 = 0;
	HRESULT hr = buffer->QueryInterface( IID_IDirectSoundBuffer8, reinterpret_cast< void** >( & buffer_8 ) );
	buffer->Release();
	
	DIRECT_X_FAIL_CHECK( hr );

	return new DirectSoundBuffer( buffer_8 );
}