#pragma once

#include "Sound.h"
#include <windows.h>

namespace core
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
	explicit StreamingSound( const DirectSound* );

	/// デストラクタ
	~StreamingSound();

	/// ファイルを読み込む
	bool load( const char* ) override;

	/// サウンドを再生する
	bool play( bool, bool ) override;
	bool is_loop() const { return is_loop_; }

	/// 更新処理
	void update() override;

	/// 現在の位置 ( 秒 ) を取得する
	float get_current_position() const override;

	/// 現在の位置のピークレベルを取得する
	float get_current_peak_level() const override;

	static DWORD get_buffer_size() { return 512 * 1024; /* 512 KB */ };

}; // class StreamingSound

} // namespace core
