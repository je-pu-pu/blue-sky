#include "AlertSystem.h"

#include <blue_sky/ai/EnemyPerception.h>

#include <core/TimeManager.h>

#include <algorithm>

namespace blue_sky::ecs
{

void AlertSystem::update( ComponentTuple& tuple ) const
{
	auto* perception = std::get< PerceptionComponent* >( tuple );
	auto* alert      = std::get< AlertComponent* >( tuple );

	const float dt = time_manager_->get_delta_time();

	// awareness を更新（視認で上昇・非視認で減衰）
	alert->awareness = ai::step_awareness(
		alert->awareness,
		perception->target_visible,
		dt,
		alert->gain_per_sec,
		alert->decay_per_sec );

	// 状態遷移（ヒステリシス付き）
	const ai::AlertState prev = alert->state;
	alert->state = ai::next_alert_state( alert->state, alert->awareness );

	// 捜索タイマ: 捜索に入ったら制限時間を張る。見失ったまま尽きたら諦めて awareness を落とす。
	if ( alert->state == ai::AlertState::Searching )
	{
		if ( prev != ai::AlertState::Searching )
		{
			alert->search_timer = alert->search_duration;
		}
		else if ( ! perception->target_visible )
		{
			alert->search_timer -= dt;
			if ( alert->search_timer <= 0.f )
			{
				alert->awareness = std::min( alert->awareness, 0.2f );
			}
		}
	}
}

} // namespace blue_sky::ecs
