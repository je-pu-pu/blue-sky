#ifndef BLUE_SKY_STREAMING_SOUND_H
#define BLUE_SKY_STREAMING_SOUND_H

#include "Sound.h"
#include <windows.h>

namespace blue_sky
{

/**
 * �X�g���[�~���O�T�E���h�N���X
 *
 */
class StreamingSound : public Sound
{
private:
	bool is_first_half_playing_;

public:

	/// �R���X�g���N�^
	StreamingSound( const DirectSound* );

	/// �f�X�g���N�^
	~StreamingSound();

	/// �t�@�C����ǂݍ���
	bool load( const char* );

	/// �X�V����
	void update();

	static DWORD get_buffer_size() { return 512 * 1024; /* 512 KB */ };

}; // class StreamingSound

} // namespace blue_sky

#endif // BLUE_SKY_STREAMING_SOUND_H
