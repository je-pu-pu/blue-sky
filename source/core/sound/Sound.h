#pragma once

#include <game/Sound.h>

#include <type/type.h>

#include <vector>
#include <string>

class DirectSound;
class DirectSoundBuffer;

class WaveFile;
class OggVorbisFile;

namespace game
{

class SoundFile;

} // namespace game

namespace core
{

/**
 * サウンドクラス
 *
 */
class Sound : public game::Sound
{
public:
	using SoundFile			= OggVorbisFile;

	using SoundSample		= s16_t;
	using SoundSampleList	= std::vector< SoundSample >;

protected:
	const DirectSound* direct_sound_;
	DirectSoundBuffer* direct_sound_buffer_;

	SoundFile* sound_file_;
	SoundSampleList sound_sample_buffer_;

	std::string name_;
	std::string file_name_;
	bool is_3d_sound_;

	T max_volume_;						///< 最大ボリューム
	T volume_fade_;						///< フェード ( 毎フレームの加算値 )

public:

	/// コンストラクタ
	explicit Sound( const DirectSound* );

	/// デストラクタ
	virtual ~Sound() override;

	/// 3D Sound
	void set_3d_sound( bool x = true ) { is_3d_sound_ = x; }
	bool is_3d_sound() const override { return is_3d_sound_; }
	
	/// ファイルを読み込む
	bool load( const char* ) override;

	/// 名前を取得する
	const char* get_name() override { return name_.c_str(); };
	void set_name( const char* name ) override { name_ = name; };

	/// ファイル名を取得する
	const char* get_file_name() const override { return file_name_.c_str(); }
	void set_file_name( const char* file_name ) override { file_name_ = file_name; };

	/// 3D 位置
	void set_3d_position( T, T, T ) override;

	/// 3D 速度
	void set_3d_velocity( T, T, T ) override;

	/// ボリューム
	T get_volume() const override;
	void set_volume( T ) override;

	/// 最大ボリューム
	T get_max_volume() const override { return max_volume_; }
	void set_max_volume( T v ) override { max_volume_ = v; set_volume( get_volume() ); }

	/// パン
	T get_pan() const override { return 0; };
	void set_pan( T ) override { };

	/// 再生スピード
	T get_speed() const override;
	void set_speed( T ) override;

	/// サウンドを再生する
	bool play( bool, bool ) override;

	/// サウンドが再生中かどうかを取得する
	bool is_playing() const override;

	/// サウンドを停止する
	bool stop() override;

	/// サウンドのフェードインを開始する
	void fade_in( T = 0.01f ) override;

	/// サウンドのフェードアウトを開始する
	void fade_out( T = 0.01f ) override;

	/// サウンドが完全にフェードインされた状態かどうかを取得する
	bool is_fade_full_in() const override;

	/// サウンドが完全にフェードアウトされた状態かどうかを取得する
	bool is_fade_full_out() const override;

	/// 現在の位置 ( 秒 ) を取得する
	float get_current_position() const override;

	void update() override;

	DirectSoundBuffer* get_direct_sound_buffer() { return direct_sound_buffer_; };
	void set_direct_sound_buffer( DirectSoundBuffer* b ) { direct_sound_buffer_ = b; }

	/// サウンドを一時停止する
	// virtual bool pause() = 0;

}; // class Sound

} // namespace core
