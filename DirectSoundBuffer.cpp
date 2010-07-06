#include "DirectSoundBuffer.h"
#include "DirectX.h"

#include <common/exception.h>

DirectSoundBuffer::DirectSoundBuffer( LPDIRECTSOUNDBUFFER buffer )
	: buffer_( buffer )
	, speed_( 1.f )
{
	caps_.dwSize = sizeof ( DSBCAPS );
	DIRECT_X_FAIL_CHECK( buffer_->GetCaps( & caps_ ) );
}

DirectSoundBuffer::~DirectSoundBuffer()
{
	buffer_->Release();
}

void DirectSoundBuffer::play( bool loop )
{
	buffer_->SetCurrentPosition( 0 );
	buffer_->Play( 0, 0, loop ? DSBPLAY_LOOPING : 0 );
}