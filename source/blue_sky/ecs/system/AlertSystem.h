#pragma once

#include <blue_sky/ecs/component/PerceptionComponent.h>
#include <blue_sky/ecs/component/AlertComponent.h>
#include <core/ecs/System.h>

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
	void update( ComponentTuple& ) const override;

}; // class AlertSystem

} // namespace blue_sky::ecs
