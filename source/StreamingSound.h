#pragma once

#include "Sound.h"
#include <windows.h>

namespace blue_sky
{

/**
 * ストリーミングサウンドクラス
 *
 */
class StreamingSound : public Sound
{
private:
	bool is_loop_;
	bool is_first_half_playing_;
	float current_position_offset_;

	void stream_all();
	void stream_half( bool );

public:

	/// コンストラクタ
	StreamingSound( const DirectSound* );

	/// デストラクタ
	~StreamingSound();

	/// ファイルを読み込む
	bool load( const char* );

	/// サウンドを再生する
	bool play( bool, bool );
	bool is_loop() const { return is_loop_; }

	/// 更新処理
	void update();

	/// 現在の位置 ( 秒 ) を取得する
	float get_current_position() const;

	/// 現在の位置のピークレベルを取得する
	float get_current_peak_level() const;

	static DWORD get_buffer_size() { return 512 * 1024; /* 512 KB */ };

}; // class StreamingSound

} // namespace blue_sky
