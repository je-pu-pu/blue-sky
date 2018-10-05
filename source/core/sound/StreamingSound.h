#pragma once

#include "Sound.h"
#include <windows.h>

namespace core
{

/**
 * �X�g���[�~���O�T�E���h�N���X
 *
 */
class StreamingSound : public Sound
{
private:
	bool is_loop_;
	bool is_first_half_playing_;
	float current_position_offset_;

	void stream_all();
	void stream_half( bool );

public:

	/// �R���X�g���N�^
	explicit StreamingSound( const DirectSound* );

	/// �f�X�g���N�^
	~StreamingSound();

	/// �t�@�C����ǂݍ���
	bool load( const char* ) override;

	/// �T�E���h���Đ�����
	bool play( bool, bool ) override;
	bool is_loop() const { return is_loop_; }

	/// �X�V����
	void update() override;

	/// ���݂̈ʒu ( �b ) ���擾����
	float get_current_position() const override;

	/// ���݂̈ʒu�̃s�[�N���x�����擾����
	float get_current_peak_level() const override;

	static DWORD get_buffer_size() { return 512 * 1024; /* 512 KB */ };

}; // class StreamingSound

} // namespace core
