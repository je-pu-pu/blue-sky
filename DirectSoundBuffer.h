#ifndef DIRECT_SOUND_BUFFER_H
#define DIRECT_SOUND_BUFFER_H

#include <dsound.h>

/**
 * DirectSoundBuffer8 のラッパークラス
 *
 */
class DirectSoundBuffer
{
private:
	LPDIRECTSOUNDBUFFER		buffer_;	///< DirectSoundBuffer

public:
	DirectSoundBuffer( LPDIRECTSOUNDBUFFER );
	~DirectSoundBuffer();
	
	void play();

	LPDIRECTSOUNDBUFFER getBuffer() { return buffer_; }

}; // class DirectSound

#endif // DIRECT_SOUND_H
