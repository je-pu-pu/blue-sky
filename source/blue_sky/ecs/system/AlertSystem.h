#pragma once

#include <blue_sky/ecs/component/PerceptionComponent.h>
#include <blue_sky/ecs/component/AlertComponent.h>
#include <core/ecs/System.h>

namespace core { class TimeManager; }

namespace blue_sky::ecs
{

using core::ecs::System;

/**
 * 警戒 System（S3 警戒状態機械）。
 *
 * 知覚結果（PerceptionComponent::target_visible）から awareness を更新し、
 * 無警戒→不審→捜索→警報の状態遷移を進める（純関数 next_alert_state を使用）。
 * 捜索状態には制限時間を持たせ、尽きたら諦めて警戒を解いていく。
 *
 * PerceptionSystem より後に走らせる。
 */
class AlertSystem : public System< PerceptionComponent, AlertComponent >
{
public:
	/// time_manager : 経過時間（作り手が渡す。以前は core::Service から取っていた）
	explicit AlertSystem( const core::TimeManager* time_manager ) : time_manager_( time_manager ) { }

	void update() override;

private:
	/// 対象 1 つぶん（update() が対象ごとに呼ぶ。仮想関数ではないので、ループの中にインライン化できる）
	void update_entity( ComponentTuple& ) const;

private:
	const core::TimeManager* time_manager_;

}; // class AlertSystem

} // namespace blue_sky::ecs
