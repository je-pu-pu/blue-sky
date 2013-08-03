//**********************************************************
/**
 * Input �̃C���^�[�t�F�C�X
 *
 * copyright (c) je-pu-pu all rights reserved
 *
 * @file		Input.h
 * @date		2006/09/12
 * @author		JE
 */
//**********************************************************


#ifndef BLUE_SKY_INPUT_H
#define BLUE_SKY_INPUT_H

#include "type.h"

#include <Windows.h>

#include <map>
#include <deque>
#include <array>
#include <vector>
#include <algorithm>

class DirectInput;

namespace game
{

class Config;

} // namespace game

namespace blue_sky
{

/**
 * ����
 *
 * @todo Windows �Ɛ؂藣��
 */
class Input
{
public:
	/**
	 * �{�^���̒�`
	 *
	 * �㉺���E�� A, B, X, Y �̃{�^��������
	 */
	enum Button { LEFT, RIGHT, UP, DOWN, A, B, JUMP, L, R, ESCAPE, MAX_BUTTONS, NONE };

	typedef game::Config Config;

	typedef std::deque< Button > ButtonStack;
	typedef std::array< unsigned int, MAX_BUTTONS > ButtonCodeList;
	typedef std::array< std::vector< int >, MAX_BUTTONS > ButtonCodeMultiList;
	typedef std::map< std::string, int > ConfigKeyCodeMap;

private:
	const DirectInput* direct_input_;						///< DirectInput

	unsigned int state_[ MAX_BUTTONS ];						///< �S�Ẵ{�^���̏��
	ButtonStack allow_stack_;								///< �ŗD��̕����{�^��

	bool joystick_enabled_;									///< �W���C�X�e�B�b�N�L���t���O

	JOYINFOEX joy_info_;									///< �W���C�X�e�B�b�N
	float joystick_axis_threshold_;							///< �W���C�X�e�B�b�N�̓��͂�臒l ( 0.f .. 1.f )

	DWORD* joystick_x_axis_pos_;							///< �W���C�X�e�B�b�N X ���W 
	DWORD* joystick_y_axis_pos_;							///< �W���C�X�e�B�b�N Y ���W 

	float joystick_x_sensitivity_;							///< �W���C�X�e�B�b�N X ���W ���x ( default : 1.f )
	float joystick_y_sensitivity_;							///< �W���C�X�e�B�b�N Y ���W ���x ( default : 1.f )

	float mouse_x_sensitivity_;								///< �}�E�X X ���W ���x ( default : 1.f )
	float mouse_y_sensitivity_;								///< �}�E�X X ���W ���x ( default : 1.f )

	float mouse_x_rate_;									///< �}�E�X X ���W ( -1.f .. 1.f )
	float mouse_y_rate_;									///< �}�E�X Y ���W ( -1.f .. 1.f )
	
	float mouse_dx_;										///< �}�E�X X �ړ���
	float mouse_dy_;

	int mouse_wheel_;										///< �}�E�X�z�C�[�� ( + : front / - : back )

	POINT mouse_point_;										///< �}�E�X ���W

	ButtonCodeMultiList key_code_;
	ButtonCodeList joystick_code_;

	ConfigKeyCodeMap config_key_code_map_;					///< �R���t�B�O�t�@�C���̃L�[�� ���� GetAsyncKeyState() �̈����ɓn���L�[�ւ̃}�b�v

	void load_key_code_config( Config&, const uint_t, const char_t*, const char_t* );
	
	void update_state_by_key_for( uint_t );

	DWORD* get_joystick_axis_pos_pointer_by_index( int );
	float get_rate_by_joystick_axis_pos( DWORD );

public:
	Input();
	~Input();

	void set_direct_input( const DirectInput* i ) { direct_input_ = i; }
	void set_mouse_x_sensitivity( float s ) { mouse_x_sensitivity_ = s; }
	void set_mouse_y_sensitivity( float s ) { mouse_y_sensitivity_ = s; }

	void load_config( Config& );

	void update();
	void update_null();
	void update_common();

	void allow_push( Button button )
	{
		allow_stack_.push_back( button );
	}

	void allow_pop( Button button )
	{
		ButtonStack::iterator i = std::find( allow_stack_.begin(), allow_stack_.end(), button );
		allow_stack_.erase( i );
	}

	/**
	 * �{�^����������Ă��Ȃ���ԂɍX�V����
	 *
	 * @param button ������Ă��Ȃ���Ԃɂ���{�^��
	 */
	void clear( Button button )
	{
		state_[ button ] = 0;
	}

	/**
	 * ���݃{�^����������Ă��邩�ǂ������擾����
	 *
	 * @param button ���ׂ�{�^��
	 * @return ���݃{�^����������Ă���ꍇ�� true ���A�����łȂ��ꍇ�� false ��Ԃ�
	 */
	bool press( Button button ) const
	{
		return ( state_[ button ] & 1 ) > 0;
	}

	/**
	 * ���������{�^���������ꂽ���ǂ������擾����
	 *
	 * @param button ���ׂ�{�^��
	 * @return ���������{�^���������ꂽ�ꍇ�� true ���A�����łȂ��ꍇ�� false ��Ԃ�
	 */
	bool push( Button button ) const
	{
		return ( state_[ button ] & 1 ) > 0 && ( state_[ button ] & 2 ) == 0;
	}

	/**
	 * ���������{�^���������ꂽ���ǂ������擾����
	 *
	 * button ���ׂ�{�^��
	 * @return ���������{�^���������ꂽ�ꍇ�� true ���A�����łȂ��ꍇ�� false ��Ԃ�
	 */
	bool release( Button button ) const
	{
		return ( state_[ button ] & 1 ) == 0 && ( state_[ button ] & 2 ) > 0;
	}

	/**
	 * ���ݍŗD��ŉ�����Ă�������{�^�����擾����
	 *
	 * @return ���ݍŗD��ŉ�����Ă�������{�^�� ( �����{�^����������Ă��Ȃ��ꍇ�� NONE ��Ԃ� )
	 */
	Button get_primary_allow_button() const
	{
		if ( allow_stack_.empty() ) return NONE;

		return allow_stack_.back();
	}

	const ButtonStack& get_allow_button_stack() const
	{
		return allow_stack_;
	}

	static const Input& get_null_input()
	{
		static Input null_input;

		return null_input;
	}

	int get_mouse_x() const { return mouse_point_.x; }
	int get_mouse_y() const { return mouse_point_.y; }

	float get_mouse_x_rate() const { return mouse_x_rate_; }
	float get_mouse_y_rate() const { return mouse_y_rate_; }

	float get_mouse_dx() const { return mouse_dx_; }
	float get_mouse_dy() const { return mouse_dy_; }

//	void set_mouse_x_rate( float );
//	void set_mouse_y_rate( float );

	void push_mouse_wheel_queue( int wheel ) { mouse_wheel_ += wheel; }
	int pop_mouse_wheel_queue() { int w = mouse_wheel_; mouse_wheel_ = 0; return w; }

}; // class Input

} // namespace blue_sky

#endif // BLUE_SKY_INPUT_H
