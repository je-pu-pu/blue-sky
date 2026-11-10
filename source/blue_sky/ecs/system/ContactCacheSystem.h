#pragma once

#include <core/ecs/System.h>
#include <blue_sky/ecs/component/ContactCacheComponent.h>

namespace blue_sky::ecs
{

/**
 * 最近触れた物体の覚え（ContactCacheComponent）を、経った時間だけ減らし、切れたら忘れる
 *
 * 以前は Ladder::update・BaseSwitch::update が自分の TimedCache を減らしていた。
 * EntityManager::update() では動かさない（本編のシーンは呼ばない）。ActiveObjectManager::update が、物体の update の後
 * （当たりを調べる前）に update_caches を呼ぶ
 */
class ContactCacheSystem : public core::ecs::System< ContactCacheComponent >
{
public:
	ContactCacheSystem() = default;

	void update() override { }		///< 何もしない（上の説明のとおり）

	/// 覚えを減らす（毎フレーム）
	void update_caches( float_t delta_time );
};

} // namespace blue_sky::ecs
