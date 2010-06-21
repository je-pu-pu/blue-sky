#ifndef BLUE_SKY_SOUND_H
#define BLUE_SKY_SOUND_H

#include <game/Sound.h>

class DirectSound;
class DirectSoundBuffer;

class WaveFile;
class OggVorbisFile;

namespace game
{

class SoundFile;

} // namespace game

namespace blue_sky
{

/**
 * サウンドクラス
 *
 */
class Sound : public game::Sound
{
public:
	// typedef WaveFile SoundFile;
	typedef OggVorbisFile SoundFile;

protected:
	const DirectSound* direct_sound_;
	DirectSoundBuffer* direct_sound_buffer_;

	SoundFile* sound_file_;

public:

	/// コンストラクタ
	Sound( const DirectSound* );

	/// デストラクタ
	virtual ~Sound();

	/// ファイルを読み込む
	bool load( const char* );

	/// ボリューム
	T get_volume();
	void set_volume( T );

	/// パン
	T get_pan() { return 0; };
	void set_pan( T ) { };

	/// 再生スピード
	T get_speed() const;
	void set_speed( T );

	/// サウンドを再生する
	bool play( bool );

	/// サウンドを停止する
	bool stop();

	DirectSoundBuffer* get_direct_sound_buffer() { return direct_sound_buffer_; };
	void set_direct_sound_buffer( DirectSoundBuffer* b ) { direct_sound_buffer_ = b; }

	/// サウンドを一時停止する
	// virtual bool pause() = 0;

}; // class Sound

} // namespace blue_sky

#endif // BLUE_SKY_SOUND_H
