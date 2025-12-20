#pragma once

namespace game
{

/**
* サウンドフォーマット
*
*/
struct SoundFormat
{
	int channels;			///< チャンネル数
	int sampling_rate;		///< サンプリング周波数
	int bit_depth;			///< 量子化ビット数

}; // class SoundFormat

} // namespace game