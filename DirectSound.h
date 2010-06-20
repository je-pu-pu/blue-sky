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
	
	DirectSoundBuffer* load_wave_file( const char* );
	// DirectSoundBuffer* load_wave_file( const char* );
	
}; // class DirectSound

#endif // DIRECT_SOUND_H
