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

//! sndPlaySound ���g�p���� windows �T�E���h�Đ��N���X
class Sound
{
private:
	std::string file_name_;

public:
	//! �R���X�g���N�^
	Sound();

	//! �f�X�g���N�^
	virtual ~Sound();

	//! �t�@�C����ǂݍ���
	virtual bool load( const char* file_name );

	//! �T�E���h���Đ�����
	virtual bool play();

	//! �T�E���h���~����
	virtual bool stop();

}; // class Sound

} // namespace win

#endif // WIN_SOUND_H

