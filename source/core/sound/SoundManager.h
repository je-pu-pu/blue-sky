#pragma once


#include <core/type.h>
#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include <windows.h>

namespace game
{

struct SoundFormat;

}

namespace core
{

class Sound;

}

namespace core::sound
{

class SoundEngine;
class SoundBuffer;
class SoundFilter;

/**
 * サウンド管理クラス
 *
 * 以前は game 層の game::SoundManager を継承していて、同じ名前のクラスが 2 つの層に並んでいた。
 * 実装はこのクラスだけなので、基底の中身（名前で引く・グループ・ミュート・音量）を取り込んで 1 つにした
 */
class SoundManager
{
public:
	using SoundFormat		= game::SoundFormat;
	using Sound				= core::Sound;
	using SoundMap			= std::unordered_map< string_t, Sound* >;
	using GroupedSoundMap	= std::unordered_map< string_t, SoundMap >;

private:
	SoundEngine* sound_engine_;
	std::vector< std::unique_ptr< SoundFilter > > sound_filter_list_;

	SoundMap sound_map_;
	string_t group_name_;
	GroupedSoundMap grouped_sound_map_;
	bool is_mute_ = false;
	float volume_ = 1.f;

	/// 一覧（sound_map_ など）を、更新スレッドとメインスレッドから守る
	mutable std::recursive_mutex mutex_;

	/// 更新スレッド（start_update_thread で動かし、stop_update_thread か破棄で止める）
	std::thread update_thread_;
	std::atomic< bool > update_thread_running_ { false };

	/// ファイルの場所を決めた後の読み込み（同じ名前で同じファイルなら使い回す）
	Sound* load_from_path( const char* name, const string_t& file_path, bool is_3d );

public:
	SoundManager( SoundEngine* );
	virtual ~SoundManager();

	SoundEngine* get_sound_engine() const { return sound_engine_; }

	bool is_mute() const { return is_mute_; }
	void set_mute( bool );

	float get_volume() const { return volume_; }
	void set_volume( float );

	/// グループ : push_group から pop_group までに読み込んだ音を、pop_group でまとめて外す
	void push_group( const char_t* group_name );
	void pop_group();

	Sound* load( const char*, const char* = 0 );
	Sound* load_3d_sound( const char*, const char* = 0 );
	Sound* load_music( const char* name, const char* file_name = 0, bool replace = false );

	void unload( const char* name );
	void unload_all();

	Sound* get_sound( const char* name ) const;

	/// フェード・ストリーミングの補充を進める（更新スレッドを動かしているときは、そのスレッドが呼ぶ）
	void update();
	void stop_all();

	/**
	 * 更新を専用のスレッドで interval_msec ごとに行う
	 *
	 * 以前はメインスレッドで毎フレーム更新していて、同期の読み込みなどでメインスレッドが止まると、
	 * ストリーミングの音（BGM）の補充が止まって音が途切れた
	 */
	void start_update_thread( unsigned int interval_msec = 16 );
	void stop_update_thread();
	bool is_update_thread_running() const { return update_thread_running_; }

	void add_sound_filter( SoundFilter* filter );

	void set_listener_position( const Vector3& );
	void set_listener_velocity( const Vector3& );
	void set_listener_orientation( const Vector3& , const Vector3& );
	void commit();

	const SoundFormat& get_format() const;

}; // class SoundManager

} // namespace core::sound
