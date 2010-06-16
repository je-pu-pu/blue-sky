#include "DirectSound.h"
#include "DirectSoundBuffer.h"

#include "WaveFile.h"
#include <common/exception.h>
#include <dxerr9.h>
#include <string>

#pragma comment( lib, "dsound.lib" )
#pragma comment( lib, "dxerr9.lib" )

DirectSound::DirectSound( HWND hwnd )
	: direct_sound_( 0 )
	, primary_buffer_( 0 )
{
	if ( FAILED( DirectSoundCreate8( NULL, & direct_sound_, 0 ) ) )
	{
		COMMON_THROW_EXCEPTION;
	}

	if ( FAILED( direct_sound_->SetCooperativeLevel( hwnd, DSSCL_PRIORITY ) ) )
	{
		COMMON_THROW_EXCEPTION;
	}

	HRESULT hr = 0;

	caps_.dwSize = sizeof( DSCAPS );
	if ( FAILED( hr = direct_sound_->GetCaps( & caps_ ) ) )
	{
		std::string message = std::string( DXGetErrorString9( hr ) ) + " : " + DXGetErrorDescription9( hr );
		COMMON_THROW_EXCEPTION_MESSAGE( message.c_str() );
	}

	// Create Primary Buffer
	DSBUFFERDESC buffer_desc = { sizeof( DSBUFFERDESC ) };
	buffer_desc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_PRIMARYBUFFER;

	if ( FAILED( direct_sound_->CreateSoundBuffer( & buffer_desc, & primary_buffer_, 0 ) ) )
	{
		COMMON_THROW_EXCEPTION;
	}

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

DirectSoundBuffer* DirectSound::load_wave_file( const char* file_name )
{
	WaveFile wave_file( file_name );

	DSBUFFERDESC buffer_desc = { sizeof( DSBUFFERDESC ) };
	buffer_desc.dwFlags = DSBCAPS_STATIC | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY;
	buffer_desc.dwBufferBytes = wave_file.size();
	buffer_desc.lpwfxFormat = & wave_file.format();

	LPDIRECTSOUNDBUFFER buffer = 0;

	if ( FAILED( direct_sound_->CreateSoundBuffer( & buffer_desc, & buffer,  0 ) ) )
	{
		COMMON_THROW_EXCEPTION;
	}

	void* data = 0;
	DWORD size = 0;

	buffer->Lock( 0, 0, & data, & size, 0, 0, DSBLOCK_ENTIREBUFFER );
	
	memcpy( data, wave_file.data(), wave_file.size() );
	
	buffer->Unlock( data, size, 0, 0 );

	return new DirectSoundBuffer( buffer );
}