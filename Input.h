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

#include <boost/array.hpp>
#include <deque>
#include <vector>
#include <algorithm>
#include <windows.h>

namespace game
{

class Config;

} // namespace game

namespace blue_sky
{

/**
 * ����
 */
class Input
{
public:
	/**
	 * �{�^���̒�`
	 *
	 * �㉺���E�� A, B, X, Y �̃{�^��������
	 */
	enum Button { LEFT, RIGHT, UP, DOWN, A, B, X, Y, L, R, MAX_BUTTONS, NONE };

	typedef game::Config Config;

	typedef std::deque< Button > ButtonStack;
	typedef boost::array< unsigned int, MAX_BUTTONS > ButtonCodeList;

private:
	unsigned int state_[ MAX_BUTTONS ];						///< �S�Ẵ{�^���̏��
	ButtonStack allow_stack_;								///< �ŗD��̕����{�^��

	JOYINFOEX joy_info_;									///< �W���C�X�e�B�b�N
	bool joystick_enabled_;									///< �W���C�X�e�B�b�N�L���t���O

	float mouse_x_;											///< �}�E�X X ���W ( -1.f .. 1.f )
	float mouse_y_;											///< �}�E�X Y ���W ( -1.f .. 1.f )
	
	float mouse_dx_;										///< �}�E�X X �ړ��� ( )
	float mouse_dy_;

	POINT last_mouse_point_;								///< �O�̃t���[���̃}�E�X ���W

	ButtonCodeList key_code_;
	ButtonCodeList joystick_code_;

public:
	Input();

	void load_config( Config& );

	void update();
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

	float get_mouse_x() const { return mouse_x_; }
	float get_mouse_y() const { return mouse_y_; }

	float get_mouse_dx() const { return mouse_dx_; }
	float get_mouse_dy() const { return mouse_dy_; }

	void set_mouse_x( float );
	void set_mouse_y( float );

}; // class Input

} // namespace blue_sky

#endif // BLUE_SKY_INPUT_H
