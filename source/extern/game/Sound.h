#ifndef GAME_SOUND_H
#define GAME_SOUND_H

namespace game
{

/**
 * �T�E���h���N���X
 *
 */
class Sound
{
public:
	typedef float T;

	static const T VOLUME_MIN;
	static const T VOLUME_MAX;

	static const T VOLUME_FADE_SPEED_DEFAULT;
	static const T VOLUME_FADE_SPEED_FAST;

	static const T PAN_LEFT;
	static const T PAN_RIGHT;
	static const T PAN_CENTER;

public:

	/// �R���X�g���N�^
	Sound() { }

	/// �f�X�g���N�^
	virtual ~Sound() { }

	/// �t�@�C����ǂݍ���
	virtual bool load( const char* ) { return true; };

	/// ���O���擾����
	virtual const char* get_name() { return ""; };
	virtual void set_name( const char* ) { };

	/// �t�@�C�������擾����
	virtual const char* get_file_name() const { return ""; }
	virtual void set_file_name( const char* ) { };

	/// 3D
	virtual bool is_3d_sound() const { return false; }

	/// 3D �ʒu
	virtual void set_3d_position( T, T, T ) { }

	/// 3D ���x
	virtual void set_3d_velocity( T, T, T ) { }

	/// �{�����[��
	virtual T get_volume() const { return 0.f; };
	virtual void set_volume( T ) { };

	/// �ő�{�����[��
	virtual T get_max_volume() const { return VOLUME_MIN; }
	virtual void set_max_volume( T ) { };

	/// �p��
	virtual T get_pan() const { return 0.f; };
	virtual void set_pan( T ) { };

	/// �Đ��X�s�[�h
	virtual T get_speed() const { return 1.f; };
	virtual void set_speed( T ) { };

	/// �T�E���h���Đ�����
	virtual bool play( bool loop, bool force = true ) { return true; };

	/// �T�E���h���Đ������ǂ������擾����
	virtual bool is_playing() const { return false; }

	/// �T�E���h���~����
	virtual bool stop() { return true; }

	/// �T�E���h�̃t�F�[�h�C�����J�n����
	virtual void fade_in( T = VOLUME_FADE_SPEED_DEFAULT ) { }

	/// �T�E���h�̃t�F�[�h�A�E�g���J�n����
	virtual void fade_out( T = VOLUME_FADE_SPEED_DEFAULT ) { }

	/// �T�E���h���ꎞ��~����
	// virtual bool pause() = 0;

	/// ���݂̈ʒu ( �b ) ���擾����
	virtual float get_current_position() const { return 0.f; }

	/// ���݂̈ʒu�̃s�[�N���x�����擾����
	virtual float get_current_peak_level() const { return 0.f; }

	/// �X�V����
	virtual void update() { };

}; // class Sound

} // namespace game

#endif // GAME_SOUND_H


/*
bgm1 = sound_manager.load( "bgm1", "bgm.wav" );
bgm2 = sound_manager.load( "bgm2", "bgm2.wav" );

a = sound_manager.load( "a", "a.wav" );
b = sound_manager.load( "b", "b.wav" );
c = sound_manager.load( "c", "c.wav" );

sound_manager.rename( "bgm2", "bgm" );

a.play();

b.set_volume( 0.f, 10 );
b.play();

c.set_volume( 0.f );
c.set_volume( 1.f, 60 );
c.set_pan( );
c.play();

sound_manager.unload( "bgm1" );
*/
