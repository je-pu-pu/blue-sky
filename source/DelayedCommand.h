#ifndef BLUE_SKY_DELAYED_COMMAND_H
#define BLUE_SKY_DELAYED_COMMAND_H

#include <type/type.h>

namespace blue_sky
{

/**
 * 遅延実行されるコマンドの設定
 *
 */
class DelayedCommand
{
public:

private:
	float_t interval_; ///< 遅延時間 ( 秒 )
	int_t loop_count_; ///< 実行回数
	string_t command_; ///< コマンド + 引数

	int_t exec_count_; ///< 実行済み回数

	float_t elapsed_; ///< オブジェクトが生成されてからの経過時間
	float_t last_exec_elapsed_; ///< 最後にコマンドを実行した時点での経過時間

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

	/// 更新
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
