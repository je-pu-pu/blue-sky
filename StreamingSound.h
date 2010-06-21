#ifndef BLUE_SKY_STREAMING_SOUND_H
#define BLUE_SKY_STREAMING_SOUND_H

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
	bool is_first_half_playing_;

public:

	/// コンストラクタ
	StreamingSound( const DirectSound* );

	/// デストラクタ
	~StreamingSound();

	/// ファイルを読み込む
	bool load( const char* );

	/// 更新処理
	void update();

	static DWORD get_buffer_size() { return 512 * 1024; /* 512 KB */ };

}; // class StreamingSound

} // namespace blue_sky

#endif // BLUE_SKY_STREAMING_SOUND_H
