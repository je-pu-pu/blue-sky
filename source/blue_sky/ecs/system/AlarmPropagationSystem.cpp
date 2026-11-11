#include "AlarmPropagationSystem.h"

#include <blue_sky/ai/EnemyPerception.h>

namespace blue_sky::ecs
{

void AlarmPropagationSystem::update()
{
	if ( ! enabled )
	{
		return;
	}

	using core::math::Vector;

	auto& list = get_component_list();

	for ( auto& src : list )
	{
		auto* src_alert = std::get< AlertComponent* >( src.second );
		if ( src_alert->state != ai::AlertState::Alarmed )
		{
			continue; // 発信源は警報中の敵のみ
		}

		auto* src_tf   = std::get< TransformComponent* >( src.second );
		auto* src_perc = std::get< PerceptionComponent* >( src.second );
		const Vector src_pos = src_tf->transform.get_position();

		for ( auto& dst : list )
		{
			if ( dst.first == src.first )
			{
				continue; // 自分自身
			}

			auto* dst_alert = std::get< AlertComponent* >( dst.second );
			if ( dst_alert->state == ai::AlertState::Alarmed )
			{
				continue; // 既に警報なら不要
			}

			auto* dst_tf = std::get< TransformComponent* >( dst.second );
			Vector d = dst_tf->transform.get_position() - src_pos;
			if ( d.length() > radius )
			{
				continue; // 伝播範囲外
			}

			// 伝播: 捜索レベルまで awareness を引き上げ、最終視認地点を共有して合流させる。
			// 次フレームの AlertSystem が awareness を状態(Searching)へ変換する。
			if ( dst_alert->awareness < 0.6f )
			{
				dst_alert->awareness = 0.6f;
			}
			if ( src_perc->has_last_seen )
			{
				auto* dst_perc = std::get< PerceptionComponent* >( dst.second );
				dst_perc->last_seen_position = src_perc->last_seen_position;
				dst_perc->has_last_seen      = true;
			}
		}
	}
}

} // namespace blue_sky::ecs
