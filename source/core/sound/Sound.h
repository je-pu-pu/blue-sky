#pragma once

#include <type/type.h>

#include <memory>
#include <mutex>
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
	namespace sound
	{
		class SoundEngine;
		class SoundBuffer;
	}

/**
 * サウンドクラス
 *
 * 以前は game 層の game::Sound（中身の無い基底）を継承していて、同じ名前のクラスが 2 つの層に並んでいた。
 * 実装はこのクラス（と StreamingSound）だけなので、基底を取り込んで 1 つにした
 */
class Sound
{
public:
	using T = float;

	static const T VOLUME_MIN;
	static const T VOLUME_MAX;

	static const T VOLUME_FADE_SPEED_DEFAULT;
	static const T VOLUME_FADE_SPEED_FAST;

	static const T PAN_LEFT;
	static const T PAN_RIGHT;
	static const T PAN_CENTER;

	using SoundFile			= OggVorbisFile;

	using SoundSample		= s16_t;
	using SoundSampleList	= std::vector< SoundSample >;

	using SoundEngine		= sound::SoundEngine;
	using SoundBuffer		= sound::SoundBuffer;

protected:
	SoundEngine* sound_engine_;
	
	std::unique_ptr< SoundBuffer > sound_buffer_;

	std::unique_ptr< SoundFile > sound_file_;
	SoundSampleList sound_sample_buffer_;

	std::string name_;
	std::string file_name_;
	bool is_3d_sound_;

	T max_volume_;						///< 最大ボリューム
	T volume_fade_;						///< フェード ( 毎フレームの加算値 )

	/// 更新（サウンドの更新スレッド）と操作（メインスレッド）を順番にする
	mutable std::recursive_mutex mutex_;

public:

	/// コンストラクタ
	explicit Sound( SoundEngine* );

	/// デストラクタ
	virtual ~Sound();

	/// 3D Sound
	void set_3d_sound( bool x = true ) { is_3d_sound_ = x; }
	virtual bool is_3d_sound() const { return is_3d_sound_; }
	
	/// ファイルを読み込む
	virtual bool load( const char* );

	/// 名前を取得する
	virtual const char* get_name() { return name_.c_str(); };
	virtual void set_name( const char* name ) { name_ = name; };

	/// ファイル名を取得する
	virtual const char* get_file_name() const { return file_name_.c_str(); }
	virtual void set_file_name( const char* file_name ) { file_name_ = file_name; };

	/// 3D 位置
	virtual void set_3d_position( T, T, T );

	/// 3D 速度
	virtual void set_3d_velocity( T, T, T );

	/// ボリューム
	virtual T get_volume() const;
	virtual void set_volume( T );

	/// 最大ボリューム
	virtual T get_max_volume() const { return max_volume_; }
	virtual void set_max_volume( T v ) { max_volume_ = v; set_volume( get_volume() ); }

	/// パン
	virtual T get_pan() const { return 0; };
	virtual void set_pan( T ) { };

	/// 再生スピード
	virtual T get_speed() const;
	virtual void set_speed( T );

	/// サウンドを再生する
	virtual bool play( bool loop, bool force = true );	///< force : 再生中でも頭から鳴らし直す（既定は以前の game::Sound と同じ true）

	/// サウンドが再生中かどうかを取得する
	virtual bool is_playing() const;

	/// サウンドを停止する
	virtual bool stop();

	/// サウンドのフェードインを開始する
	virtual void fade_in( T = 0.01f );

	/// サウンドのフェードアウトを開始する
	virtual void fade_out( T = 0.01f );

	/// サウンドが完全にフェードインされた状態かどうかを取得する
	virtual bool is_fade_full_in() const;

	/// サウンドが完全にフェードアウトされた状態かどうかを取得する
	virtual bool is_fade_full_out() const;

	/// 現在の位置 ( 秒 ) を取得する
	virtual float get_current_position() const;

	/// 現在の位置のピークレベルを取得する（StreamingSound が上書きする）
	virtual float get_current_peak_level() const { return 0.f; }

	virtual void update();

	/// サウンドを一時停止する
	// virtual bool pause() = 0;

}; // class Sound

} // namespace core
