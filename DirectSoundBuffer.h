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

	float speed_; ///< 
public:
	DirectSoundBuffer( LPDIRECTSOUNDBUFFER );
	~DirectSoundBuffer();
	
	void play( bool = false );

	LPDIRECTSOUNDBUFFER get_direct_sound_buffer() { return buffer_; }

	float getSpeed() const { return speed_; }
	void setSpeed( float s ) { speed_ = s; buffer_->SetFrequency( DSBFREQUENCY_ORIGINAL ); setFrequency( static_cast< DWORD >( getFrequency() * speed_ ) ); }

	DWORD getFrequency() const { DWORD f; buffer_->GetFrequency( & f ); return f; }
	void setFrequency( DWORD f ) { buffer_->SetFrequency( f ); }


}; // class DirectSound

#endif // DIRECT_SOUND_H
