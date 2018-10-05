#pragma once

#include <game/Sound.h>

#include <type/type.h>

#include <vector>
#include <string>

class DirectSound;
class DirectSoundBuffer;

class WaveFile;
class OggVorbisFile;

namespace game
{

class SoundFile;

} // namespace game

namespace core
{

/**
 * �T�E���h�N���X
 *
 */
class Sound : public game::Sound
{
public:
	using SoundFile			= OggVorbisFile;

	using SoundSample		= s16_t;
	using SoundSampleList	= std::vector< SoundSample >;

protected:
	const DirectSound* direct_sound_;
	DirectSoundBuffer* direct_sound_buffer_;

	SoundFile* sound_file_;
	SoundSampleList sound_sample_buffer_;

	std::string name_;
	std::string file_name_;
	bool is_3d_sound_;

	T max_volume_;						///< �ő�{�����[��
	T volume_fade_;						///< �t�F�[�h ( ���t���[���̉��Z�l )

public:

	/// �R���X�g���N�^
	explicit Sound( const DirectSound* );

	/// �f�X�g���N�^
	virtual ~Sound() override;

	/// 3D Sound
	void set_3d_sound( bool x = true ) { is_3d_sound_ = x; }
	bool is_3d_sound() const override { return is_3d_sound_; }
	
	/// �t�@�C����ǂݍ���
	bool load( const char* ) override;

	/// ���O���擾����
	const char* get_name() override { return name_.c_str(); };
	void set_name( const char* name ) override { name_ = name; };

	/// �t�@�C�������擾����
	const char* get_file_name() const override { return file_name_.c_str(); }
	void set_file_name( const char* file_name ) override { file_name_ = file_name; };

	/// 3D �ʒu
	void set_3d_position( T, T, T ) override;

	/// 3D ���x
	void set_3d_velocity( T, T, T ) override;

	/// �{�����[��
	T get_volume() const override;
	void set_volume( T ) override;

	/// �ő�{�����[��
	T get_max_volume() const override { return max_volume_; }
	void set_max_volume( T v ) override { max_volume_ = v; set_volume( get_volume() ); }

	/// �p��
	T get_pan() const override { return 0; };
	void set_pan( T ) override { };

	/// �Đ��X�s�[�h
	T get_speed() const override;
	void set_speed( T ) override;

	/// �T�E���h���Đ�����
	bool play( bool, bool ) override;

	/// �T�E���h���Đ������ǂ������擾����
	bool is_playing() const override;

	/// �T�E���h���~����
	bool stop() override;

	/// �T�E���h�̃t�F�[�h�C�����J�n����
	void fade_in( T = 0.01f ) override;

	/// �T�E���h�̃t�F�[�h�A�E�g���J�n����
	void fade_out( T = 0.01f ) override;

	/// �T�E���h�����S�Ƀt�F�[�h�C�����ꂽ��Ԃ��ǂ������擾����
	bool is_fade_full_in() const override;

	/// �T�E���h�����S�Ƀt�F�[�h�A�E�g���ꂽ��Ԃ��ǂ������擾����
	bool is_fade_full_out() const override;

	/// ���݂̈ʒu ( �b ) ���擾����
	float get_current_position() const override;

	void update() override;

	DirectSoundBuffer* get_direct_sound_buffer() { return direct_sound_buffer_; };
	void set_direct_sound_buffer( DirectSoundBuffer* b ) { direct_sound_buffer_ = b; }

	/// �T�E���h���ꎞ��~����
	// virtual bool pause() = 0;

}; // class Sound

} // namespace core
