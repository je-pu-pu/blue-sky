//**********************************************************
//! copyright (c) JE all rights reserved
//!
//! \file		Sound.cpp
//! \date		2003/05/20
//! \author		JE
//! \brief		Sound の実装
//**********************************************************

#include "Sound.h"
#include <windows.h>
#include <mmsystem.h>

namespace win
{

//! コンストラクタ
Sound::Sound()
{

}

//! デストラクタ
Sound::~Sound()
{
	
}

//! ファイルを読み込む
bool Sound::load( const char* file_name )
{
	file_name_ = file_name;
	return true;
}

//! サウンドを再生する
bool Sound::play()
{
	return sndPlaySoundA( file_name_.c_str(), SND_ASYNC ) != 0;
}

//! サウンドを停止する
bool Sound::stop()
{
	return false;
}

} // namespace win


