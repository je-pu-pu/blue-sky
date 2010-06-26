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

#include <deque>
#include <algorithm>
#include <windows.h>

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

	typedef std::deque< Button > ButtonStack;

private:
	unsigned int state_[ MAX_BUTTONS ];						///< �S�Ẵ{�^���̏��
	ButtonStack allow_stack_;								///< �ŗD��̕����{�^��

	JOYINFOEX joy_info_;									///< �W���C�X�e�B�b�N
	bool joystick_enabled_;									///< �W���C�X�e�B�b�N�L���t���O

public:
	Input();

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
	Button getPrimaryAllowButton() const
	{
		if ( allow_stack_.empty() ) return NONE;

		return allow_stack_.back();
	}

	const ButtonStack& getAllowButtonStack() const
	{
		return allow_stack_;
	}

	static const Input& GetNullInput()
	{
		static Input null_input;

		return null_input;
	}
}; // class Input

} // namespace blue_sky

#endif // BLUE_SKY_INPUT_H
