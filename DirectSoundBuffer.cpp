#include "DirectSoundBuffer.h"

DirectSoundBuffer::DirectSoundBuffer( LPDIRECTSOUNDBUFFER buffer )
	: buffer_( buffer )
{

}

DirectSoundBuffer::~DirectSoundBuffer()
{
	buffer_->Release();
}

void DirectSoundBuffer::play()
{
	buffer_->Play( 0, 0, 0 );
}