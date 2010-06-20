#ifndef BLUE_SKY_STREAMING_SOUND_H
#define BLUE_SKY_STREAMING_SOUND_H

#include "Sound.h"

namespace blue_sky
{

/**
 * �X�g���[�~���O�T�E���h�N���X
 *
 */
class StreamingSound : public Sound
{
public:

	/// �R���X�g���N�^
	StreamingSound( const DirectSound* );

	/// �f�X�g���N�^
	~StreamingSound();

	/// �t�@�C����ǂݍ���
	bool load( const char* );

}; // class StreamingSound

} // namespace blue_sky

#endif // BLUE_SKY_STREAMING_SOUND_H
