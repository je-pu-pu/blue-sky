#ifndef BLUE_SKY_DELAYED_COMMAND_H
#define BLUE_SKY_DELAYED_COMMAND_H

#include <type/type.h>

namespace blue_sky
{

/**
 * �x�����s�����R�}���h�̐ݒ�
 *
 */
class DelayedCommand
{
public:

private:
	float_t interval_; ///< �x������ ( �b )
	int_t loop_count_; ///< ���s��
	string_t command_; ///< �R�}���h + ����

	int_t exec_count_; ///< ���s�ς݉�

	float_t elapsed_; ///< �I�u�W�F�N�g����������Ă���̌o�ߎ���
	float_t last_exec_elapsed_; ///< �Ō�ɃR�}���h�����s�������_�ł̌o�ߎ���

public:
	DelayedCommand( float_t interval, int loop_count, const string_t& command )
		: interval_( interval )
		, loop_count_( loop_count )
		, command_( command )
		, exec_count_( 0 )
		, elapsed_( 0.f )
		, last_exec_elapsed_( 0.f )
	{

	}

	/// �X�V
	bool update( float_t elapsed_time )
	{
		elapsed_ += elapsed_time;
		
		if ( elapsed_ - last_exec_elapsed_ >= interval_ )
		{
			last_exec_elapsed_ = elapsed_;
			exec_count_++;

			return true;
		}

		return false;
	}

	float_t get_interval() const { return interval_; }
	int_t get_loop_count() const { return loop_count_; }
	const string_t& get_command() const { return command_; }


	bool is_over() const { return exec_count_ >= loop_count_; }

}; // class Switch

} // namespace blue_sky

#endif // BLUE_SKY_SWITCH_H
