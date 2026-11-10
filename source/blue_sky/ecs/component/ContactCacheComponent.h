#pragma once

#include <core/ecs/Component.h>
#include <core/ecs/type.h>
#include <game/TimedCache.h>

namespace blue_sky::ecs
{

/**
 * 最近触れた物体を、しばらく覚えておく（触れ続けている間に何度も反応しないための間引き）
 *
 * 以前は Ladder・BaseSwitch がそれぞれ TimedCache< const GameObject* > を持ち、自分の update で減らしていた。
 * 今は触れた物体を Entity の番号で覚え、ContactCacheSystem が毎フレーム減らす。やり直すと忘れる（EntityObject::restart）
 */
struct ContactCacheComponent : public core::ecs::Component
{
	game::TimedCache< core::ecs::EntityId > cache;		///< 触れた物体 → 残りの秒数

	/// 覚えておく秒数（以前の梯子・スイッチと同じ 0.5 秒）
	static constexpr float_t LifeTime = 0.5f;
};

/**
 * 梯子の印
 *
 * 以前の Ladder クラスの代わり。当たりの互換の層（ActiveObjectManager::on_collide_with_entity_object）が、
 * これを持つ Entity に当たった物体の GameObject::on_collide_with_ladder を呼び、反応したら（true なら）、
 * 最近触れていなければ梯子の位置で "ladder-contact" を鳴らして、触れた物体を ContactCacheComponent に覚える
 */
struct LadderComponent : public core::ecs::Component
{

};

} // namespace blue_sky::ecs
