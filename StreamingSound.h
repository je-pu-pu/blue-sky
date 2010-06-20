#ifndef BLUE_SKY_STREAMING_SOUND_H
#define BLUE_SKY_STREAMING_SOUND_H

#include "Sound.h"

namespace blue_sky
{

/**
 * ストリーミングサウンドクラス
 *
 */
class StreamingSound : public Sound
{
public:

	/// コンストラクタ
	StreamingSound( const DirectSound* );

	/// デストラクタ
	~StreamingSound();

	/// ファイルを読み込む
	bool load( const char* );

}; // class StreamingSound

} // namespace blue_sky

#endif // BLUE_SKY_STREAMING_SOUND_H
