#ifndef GAME_SOUND_FILE_H
#define GAME_SOUND_FILE_H

namespace game
{

/**
 * サウンドファイル基底クラス
 *
 */
class SoundFile
{
public:
	using SizeType = unsigned long;

	/*
	struct Info
	{
		unsigned short channels;
		unsigned short sampling_rate;
		unsigned short 
	};
	*/

public:

	/// コンストラクタ
	SoundFile() { }

	/// デストラクタ
	virtual ~SoundFile() { }

	/// サイズを取得する
	virtual SizeType size() const = 0;

	/// 1 秒あたりのサイズを取得する
	virtual SizeType size_per_sec() const = 0;

	/// ファイルからデータを読み込む
	virtual SizeType read( void*, SizeType, bool ) = 0;



}; // class SoundFile

} // namespace game

#endif // GAME_SOUND_FILE_H
