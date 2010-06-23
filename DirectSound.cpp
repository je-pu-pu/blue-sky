#include "DirectSound.h"
#include "DirectSoundBuffer.h"
#include "DirectX.h"

#include <common/exception.h>
#include <string>

#pragma comment( lib, "dsound.lib" )
#pragma comment( lib, "dxerr9.lib" )

DirectSound::DirectSound( HWND hwnd )
	: direct_sound_( 0 )
	, primary_buffer_( 0 )
{
	DIRECT_X_FAIL_CHECK( DirectSoundCreate8( NULL, & direct_sound_, 0 ) );
	DIRECT_X_FAIL_CHECK( direct_sound_->SetCooperativeLevel( hwnd, DSSCL_PRIORITY ) );

	caps_.dwSize = sizeof( DSCAPS );
	DIRECT_X_FAIL_CHECK( direct_sound_->GetCaps( & caps_ ) );

	// Create Primary Buffer
	DSBUFFERDESC buffer_desc = { sizeof( DSBUFFERDESC ) };
	buffer_desc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_PRIMARYBUFFER;

	DIRECT_X_FAIL_CHECK( direct_sound_->CreateSoundBuffer( & buffer_desc, & primary_buffer_, 0 ) );

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

	if ( FAILED( direct_sound_->CreateSoundBuffer( & buffer_desc, & buffer,  0 ) ) )
	{
		COMMON_THROW_EXCEPTION;
	}

	return new DirectSoundBuffer( buffer );
}

