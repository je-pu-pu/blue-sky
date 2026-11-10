#pragma once

#include <core/ecs/Component.h>
#include <core/type.h>

namespace blue_sky::ecs
{

/**
 * 目標地点（着いたら MoveToTargetSystem が外す）
 *
 * 以前は ActiveObjectManager の unordered_map< ActiveObject*, tuple< Vector, float_t > > が持っていた
 */
struct TargetLocationComponent : public core::ecs::Component
{
	core::Vector	location;		///< 目標地点
	float_t			speed = 0.f;	///< 速さ（1 秒に進む距離）
};

} // namespace blue_sky::ecs
