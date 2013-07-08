//**********************************************************
//! copyright (c) JE all rights reserved
//!
//! \file		Sound.cpp
//! \date		2003/05/20
//! \author		JE
//! \brief		Sound �̎���
//**********************************************************

#include "Sound.h"
#include <windows.h>
#include <mmsystem.h>

namespace win
{

//! �R���X�g���N�^
Sound::Sound()
{

}

//! �f�X�g���N�^
Sound::~Sound()
{
	
}

//! �t�@�C����ǂݍ���
bool Sound::load( const char* file_name )
{
	file_name_ = file_name;
	return true;
}

//! �T�E���h���Đ�����
bool Sound::play()
{
	return sndPlaySoundA( file_name_.c_str(), SND_ASYNC ) != 0;
}

//! �T�E���h���~����
bool Sound::stop()
{
	return false;
}

} // namespace win


