#ifndef BLUE_SKY_SOUND_H
#define BLUE_SKY_SOUND_H

#include <game/Sound.h>
#include <string>

class DirectSound;
class DirectSoundBuffer;

class WaveFile;
class OggVorbisFile;

namespace game
{

class SoundFile;

} // namespace game

namespace blue_sky
{

/**
 * �T�E���h�N���X
 *
 */
class Sound : public game::Sound
{
public:
	// typedef WaveFile SoundFile;
	typedef OggVorbisFile SoundFile;

protected:
	const DirectSound* direct_sound_;
	DirectSoundBuffer* direct_sound_buffer_;

	SoundFile* sound_file_;

	std::string name_;
	std::string file_name_;

public:

	/// �R���X�g���N�^
	Sound( const DirectSound* );

	/// �f�X�g���N�^
	virtual ~Sound();

	/// �t�@�C����ǂݍ���
	bool load( const char* );

	/// ���O���擾����
	const char* get_name() { return name_.c_str(); };
	void set_name( const char* name ) { name_ = name; };

	/// �t�@�C�������擾����
	const char* get_file_name() const { return file_name_.c_str(); }
	void set_file_name( const char* file_name ) { file_name_ = file_name; };

	/// �{�����[��
	T get_volume();
	void set_volume( T );

	/// �p��
	T get_pan() { return 0; };
	void set_pan( T ) { };

	/// �Đ��X�s�[�h
	T get_speed() const;
	void set_speed( T );

	/// �T�E���h���Đ�����
	bool play( bool );

	/// �T�E���h���~����
	bool stop();

	/// ���݂̈ʒu ( �b ) ���擾����
	float get_current_position() const;

	DirectSoundBuffer* get_direct_sound_buffer() { return direct_sound_buffer_; };
	void set_direct_sound_buffer( DirectSoundBuffer* b ) { direct_sound_buffer_ = b; }

	/// �T�E���h���ꎞ��~����
	// virtual bool pause() = 0;

}; // class Sound

} // namespace blue_sky

#endif // BLUE_SKY_SOUND_H
