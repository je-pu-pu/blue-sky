#pragma once

namespace core
{

/**
 * 時間管理クラス
 *
 * ゲーム内の時間情報を提供する
 */
class TimeManager
{
public:
	virtual ~TimeManager() = default;

	/// フレーム間の経過時間 (秒)
	virtual float get_delta_time() const = 0;

	/// ゲーム開始からの総経過時間 (秒)
	virtual float get_total_elapsed_time() const = 0;

}; // class TimeManager

} // namespace core
