#ifndef DIRECT_SOUND_H
#define DIRECT_SOUND_H

// #define DIRECTSOUND_VERSION 0x1000
#include <dsound.h>

class DirectSoundBuffer;

/**
 * DirectSound のラッパークラス
 *
 */
class DirectSound
{
private:
	LPDIRECTSOUND8			direct_sound_;		///< DirectSound 8
	LPDIRECTSOUNDBUFFER		primary_buffer_;	///< Primary Buffer

	DSCAPS					caps_;				///< Caps

public:
	DirectSound( HWND );
	~DirectSound();
	
	DirectSoundBuffer* create_sound_buffer( const DSBUFFERDESC& ) const;
	DirectSoundBuffer* create_streaming_sound_buffer( const DSBUFFERDESC& ) const;

	LPDIRECTSOUND8 get_direct_sound() const { return direct_sound_; }
	// LPDIRECTSOUNDBUFFER get_primary_buffer() { return primary_buffer_; }

}; // class DirectSound

#endif // DIRECT_SOUND_H
