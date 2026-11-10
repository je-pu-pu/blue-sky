#pragma once

#include <core/ecs/Component.h>
#include <core/ecs/type.h>
#include <core/type.h>

namespace blue_sky::ecs
{

/**
 * 目標の向き（決まった向き か ほかの物体の方向。着いても外さず、ステージをやり直すまで追い続ける）
 *
 * 以前は ActiveObjectManager の 2 つの unordered_map（向き・物体）が持ち、片方を設定するともう片方を消していた。
 * ここでは 1 つの部品にまとめ、どちらを追うかを is_entity_target で表す
 */
struct TargetDirectionComponent : public core::ecs::Component
{
	bool					is_entity_target = false;	///< true なら target_entity の方向を、false なら degree を追う
	float_t					degree = 0.f;				///< 目標の向き（Y 軸まわりの度）
	core::ecs::EntityId		target_entity = 0;			///< 向く先の物体（消えたらこの部品を外す）
	float_t					speed = 0.f;				///< 向きを変える速さ
};

} // namespace blue_sky::ecs
