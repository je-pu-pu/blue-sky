#pragma once

#include <core/TimeManager.h>

namespace blue_sky
{

/**
 * TimeManager の具体的な実装
 *
 * GameMain が所有し、毎フレーム更新する
 */
class TimeManager : public core::TimeManager
{
private:
	float elapsed_time_ = 0.f;			///< フレーム間の経過時間 (秒)
	float total_elapsed_time_ = 0.f;	///< ゲーム開始からの総経過時間 (秒)

public:
	/// フレーム間の経過時間 (秒)
	float get_delta_time() const override { return elapsed_time_; }

	/// ゲーム開始からの総経過時間 (秒)
	float get_total_elapsed_time() const override { return total_elapsed_time_; }

	/// 毎フレーム呼び出す
	void update( float elapsed_time )
	{
		elapsed_time_ = elapsed_time;
		total_elapsed_time_ += elapsed_time;
	}

	/// 総経過時間をリセット
	void reset_total_elapsed_time() { total_elapsed_time_ = 0.f; }

}; // class TimeManager

} // namespace blue_sky
