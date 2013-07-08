//! *********************************************************
//! copyright (c) JE all rights reserved
//!
//! \file		Sound.h
//! \date		2003/05/20
//! \author		JE
//! *********************************************************

#ifndef WIN_SOUND_H
#define WIN_SOUND_H

#include <string>

namespace win
{

//! sndPlaySound を使用する windows サウンド再生クラス
class Sound
{
private:
	std::string file_name_;

public:
	//! コンストラクタ
	Sound();

	//! デストラクタ
	virtual ~Sound();

	//! ファイルを読み込む
	virtual bool load( const char* file_name );

	//! サウンドを再生する
	virtual bool play();

	//! サウンドを停止する
	virtual bool stop();

}; // class Sound

} // namespace win

#endif // WIN_SOUND_H

