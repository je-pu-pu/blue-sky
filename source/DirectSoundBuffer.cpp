#include "DirectSoundBuffer.h"
#include "DirectX.h"

#include <common/exception.h>

#include "memory.h"

DirectSoundBuffer::DirectSoundBuffer( LPDIRECTSOUNDBUFFER8 buffer )
	: buffer_( buffer )
	, buffer_3d_( 0 )
	, speed_( 1.f )
{
	caps_.dwSize = sizeof ( DSBCAPS );
	DIRECT_X_FAIL_CHECK( buffer_->GetCaps( & caps_ ) );
}

DirectSoundBuffer::~DirectSoundBuffer()
{
	if ( buffer_3d_ )
	{
		buffer_3d_->Release();
	}

	buffer_->Release();
}

void DirectSoundBuffer::set_3d_sound( bool is_3d_sound )
{
	if ( is_3d_sound && ! buffer_3d_ )
	{
		DIRECT_X_FAIL_CHECK( buffer_->QueryInterface( IID_IDirectSound3DBuffer8, reinterpret_cast< void** >( & buffer_3d_ ) ) );
	}
}

void DirectSoundBuffer::play( bool loop )
{
	buffer_->SetCurrentPosition( 0 );
	buffer_->Play( 0, 0, loop ? DSBPLAY_LOOPING : 0 );
}