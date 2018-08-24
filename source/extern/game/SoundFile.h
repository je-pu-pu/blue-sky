#ifndef GAME_SOUND_FILE_H
#define GAME_SOUND_FILE_H

namespace game
{

/**
 * �T�E���h�t�@�C�����N���X
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

	/// �R���X�g���N�^
	SoundFile() { }

	/// �f�X�g���N�^
	virtual ~SoundFile() { }

	/// �T�C�Y���擾����
	virtual SizeType size() const = 0;

	/// 1 �b������̃T�C�Y���擾����
	virtual SizeType size_per_sec() const = 0;

	/// �t�@�C������f�[�^��ǂݍ���
	virtual SizeType read( void*, SizeType, bool ) = 0;



}; // class SoundFile

} // namespace game

#endif // GAME_SOUND_FILE_H
