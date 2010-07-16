#ifndef BLUE_SKY_SOUND_H
#define BLUE_SKY_SOUND_H

#include <game/Sound.h>
#include <string>

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

	std::string name_;
	std::string file_name_;
	bool is_3d_sound_;

public:

	/// コンストラクタ
	Sound( const DirectSound* );

	/// デストラクタ
	virtual ~Sound();

	/// 3D Sound
	void set_3d_sound( bool x = true ) { is_3d_sound_ = x; }
	bool is_3d_sound() const { return is_3d_sound_; }
	
	/// ファイルを読み込む
	bool load( const char* );

	/// 名前を取得する
	const char* get_name() { return name_.c_str(); };
	void set_name( const char* name ) { name_ = name; };

	/// ファイル名を取得する
	const char* get_file_name() const { return file_name_.c_str(); }
	void set_file_name( const char* file_name ) { file_name_ = file_name; };

	/// 3D 位置
	void set_3d_position( T, T, T );

	/// 3D 速度
	void set_3d_velocity( T, T, T );

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

	/// 現在の位置 ( 秒 ) を取得する
	float get_current_position() const;

	DirectSoundBuffer* get_direct_sound_buffer() { return direct_sound_buffer_; };
	void set_direct_sound_buffer( DirectSoundBuffer* b ) { direct_sound_buffer_ = b; }

	/// サウンドを一時停止する
	// virtual bool pause() = 0;

}; // class Sound

} // namespace blue_sky

#endif // BLUE_SKY_SOUND_H
