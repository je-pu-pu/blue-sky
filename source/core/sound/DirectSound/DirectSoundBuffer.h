#ifndef DIRECT_SOUND_BUFFER_H
#define DIRECT_SOUND_BUFFER_H

#define DIRECTSOUND_VERSION 0x1000
#include <dsound.h>

/**
 * DirectSoundBuffer8 のラッパークラス
 *
 */
class DirectSoundBuffer
{
private:
	LPDIRECTSOUNDBUFFER8	buffer_;	///< DirectSoundBuffer
	LPDIRECTSOUND3DBUFFER8	buffer_3d_;	///< DirectSound3DBuffer

	DSBCAPS					caps_;		///< Caps

	float speed_; ///< 
public:
	explicit DirectSoundBuffer( LPDIRECTSOUNDBUFFER8 );
	~DirectSoundBuffer();
	
	void set_3d_sound( bool );

	void play( bool = false );

	LPDIRECTSOUNDBUFFER8 get_direct_sound_buffer() { return buffer_; }
	LPDIRECTSOUND3DBUFFER8 get_direct_sound_3d_buffer() { return buffer_3d_; }

	DSBCAPS& get_caps() { return caps_; }

	float getSpeed() const { return speed_; }
	void setSpeed( float s ) { speed_ = s; buffer_->SetFrequency( DSBFREQUENCY_ORIGINAL ); setFrequency( static_cast< DWORD >( getFrequency() * speed_ ) ); }

	DWORD getFrequency() const { DWORD f; buffer_->GetFrequency( & f ); return f; }
	void setFrequency( DWORD f ) { buffer_->SetFrequency( f ); }


}; // class DirectSound

#endif // DIRECT_SOUND_H
