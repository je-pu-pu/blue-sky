#pragma once

namespace game
{
	class SoundFormat;

/**
 * サウンドファイル基底クラス
 *
 */
class SoundFile
{
public:
	using SizeType = unsigned long;
	using SoundFormat = SoundFormat;

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

	/// サウンドファイルのフォーマットを取得する
	virtual const SoundFormat& format() const = 0;

}; // class SoundFile

} // namespace game
