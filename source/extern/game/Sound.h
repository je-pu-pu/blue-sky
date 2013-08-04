#ifndef GAME_SOUND_H
#define GAME_SOUND_H

namespace game
{

/**
 * サウンド基底クラス
 *
 */
class Sound
{
public:
	typedef float T;

	static const T VOLUME_MIN;
	static const T VOLUME_MAX;

	static const T VOLUME_FADE_SPEED_DEFAULT;
	static const T VOLUME_FADE_SPEED_FAST;

	static const T PAN_LEFT;
	static const T PAN_RIGHT;
	static const T PAN_CENTER;

public:

	/// コンストラクタ
	Sound() { }

	/// デストラクタ
	virtual ~Sound() { }

	/// ファイルを読み込む
	virtual bool load( const char* ) { return true; };

	/// 名前を取得する
	virtual const char* get_name() { return ""; };
	virtual void set_name( const char* ) { };

	/// ファイル名を取得する
	virtual const char* get_file_name() const { return ""; }
	virtual void set_file_name( const char* ) { };

	/// 3D
	virtual bool is_3d_sound() const { return false; }

	/// 3D 位置
	virtual void set_3d_position( T, T, T ) { }

	/// 3D 速度
	virtual void set_3d_velocity( T, T, T ) { }

	/// ボリューム
	virtual T get_volume() const { return 0.f; };
	virtual void set_volume( T ) { };

	/// 最大ボリューム
	virtual T get_max_volume() const { return VOLUME_MIN; }
	virtual void set_max_volume( T ) { };

	/// パン
	virtual T get_pan() const { return 0.f; };
	virtual void set_pan( T ) { };

	/// 再生スピード
	virtual T get_speed() const { return 1.f; };
	virtual void set_speed( T ) { };

	/// サウンドを再生する
	virtual bool play( bool loop, bool force = true ) { return true; };

	/// サウンドが再生中かどうかを取得する
	virtual bool is_playing() const { return false; }

	/// サウンドを停止する
	virtual bool stop() { return true; }

	/// サウンドのフェードインを開始する
	virtual void fade_in( T = VOLUME_FADE_SPEED_DEFAULT ) { }

	/// サウンドのフェードアウトを開始する
	virtual void fade_out( T = VOLUME_FADE_SPEED_DEFAULT ) { }

	/// サウンドを一時停止する
	// virtual bool pause() = 0;

	/// 現在の位置 ( 秒 ) を取得する
	virtual float get_current_position() const { return 0.f; }

	/// 現在の位置のピークレベルを取得する
	virtual float get_current_peak_level() const { return 0.f; }

	/// 更新処理
	virtual void update() { };

}; // class Sound

} // namespace game

#endif // GAME_SOUND_H


/*
bgm1 = sound_manager.load( "bgm1", "bgm.wav" );
bgm2 = sound_manager.load( "bgm2", "bgm2.wav" );

a = sound_manager.load( "a", "a.wav" );
b = sound_manager.load( "b", "b.wav" );
c = sound_manager.load( "c", "c.wav" );

sound_manager.rename( "bgm2", "bgm" );

a.play();

b.set_volume( 0.f, 10 );
b.play();

c.set_volume( 0.f );
c.set_volume( 1.f, 60 );
c.set_pan( );
c.play();

sound_manager.unload( "bgm1" );
*/
