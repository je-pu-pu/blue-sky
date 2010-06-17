#include "DirectSoundBuffer.h"

DirectSoundBuffer::DirectSoundBuffer( LPDIRECTSOUNDBUFFER buffer )
	: buffer_( buffer )
	, speed_( 1.f )
{

}

DirectSoundBuffer::~DirectSoundBuffer()
{
	buffer_->Release();
}

void DirectSoundBuffer::play( bool loop )
{
	buffer_->Play( 0, 0, loop ? DSBPLAY_LOOPING : 0 );
}