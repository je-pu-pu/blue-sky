#pragma once

#include <core/ecs/Component.h>
#include <blue_sky/ai/EnemyPerception.h>

namespace blue_sky::ecs
{

/**
 * 敵の警戒コンポーネント（S3 警戒状態機械）。
 *
 * awareness（発見度 [0,1]）と現在の警戒状態（無警戒→不審→捜索→警報）を保持する。
 * 警戒 System が知覚結果から awareness を更新し、状態を遷移させる。
 */
struct AlertComponent : public core::ecs::Component
{
	/// 発見度 [0,1]。視認で上昇・非視認で減衰。
	float awareness = 0.f;

	/// 現在の警戒状態
	ai::AlertState state = ai::AlertState::Unaware;

	/// 視認中の awareness 上昇レート (/s)。1.5 なら約 0.67s でフル発見。
	float gain_per_sec = 1.5f;

	/// 非視認中の awareness 減衰レート (/s)。上昇よりゆるやかにして警戒を尾を引かせる。
	float decay_per_sec = 0.3f;

	/// 捜索状態の残り時間 (s)。0 になると諦めて（awareness を落として）警戒を解く。
	float search_timer = 0.f;

	/// 捜索を開始したときにセットする制限時間 (s)。
	float search_duration = 8.f;

}; // struct AlertComponent

} // namespace blue_sky::ecs
