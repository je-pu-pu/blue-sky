#pragma once

#include <core/type.h>

namespace core::ui
{

/**
 * キーリピート判定
 *
 * ボタンの押下状態と経過時間を毎フレーム渡すと、「このフレームで操作を 1 回発火するか」を返す。
 * 押した瞬間に 1 回、その後 initial_delay 秒押し続けると、以後 interval 秒ごとに発火する。
 *
 * プラットフォームの入力 API に依存しないため、そのままテストできる。
 */
class NavRepeater
{
private:
	float_t initial_delay_;
	float_t interval_;

	bool was_down_ = false;
	float_t held_time_ = 0.f;	///< 押し始めからの経過時間
	float_t next_fire_ = 0.f;	///< 次に発火する held_time_

public:
	explicit NavRepeater( float_t initial_delay = 0.35f, float_t interval = 0.08f )
		: initial_delay_( initial_delay )
		, interval_( interval )
	{ }

	/**
	 * 1 フレーム分更新する
	 *
	 * @param is_down 現在押されているか
	 * @param dt 前フレームからの経過秒数
	 * @return このフレームで発火するなら true
	 */
	bool update( bool is_down, float_t dt )
	{
		if ( ! is_down )
		{
			reset();
			return false;
		}

		if ( ! was_down_ )
		{
			was_down_ = true;
			held_time_ = 0.f;
			next_fire_ = initial_delay_;
			return true;
		}

		held_time_ += dt;

		if ( held_time_ >= next_fire_ )
		{
			// 通常は一定周期で発火する。
			// 1 フレームが長く周期を飛び越えた場合は 1 回だけ発火し、取りこぼした分は捨てて今から interval 後を次回とする
			next_fire_ += interval_;

			if ( next_fire_ <= held_time_ )
			{
				next_fire_ = held_time_ + interval_;
			}

			return true;
		}

		return false;
	}

	void reset()
	{
		was_down_ = false;
		held_time_ = 0.f;
		next_fire_ = 0.f;
	}

}; // class NavRepeater

} // namespace core::ui
