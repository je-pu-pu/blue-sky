#pragma once

#include <core/ecs/Component.h>
#include <core/type.h>

namespace blue_sky::ecs
{

/**
 * ゆらぎ（ふわふわと上下する動き）の大きさ（以前の ActiveObject::flicker_scale_）
 *
 * ステージ・スクリプトの set_flicker（GameObjectRef::set_flicker_scale）が書き、少女・敵が浮くときに使う。やり直すと 1 に戻る。
 * 持たない物体に set_flicker するとログを残して何もしない（風船はゆらぎの大きさが決まっているので持たない）
 */
struct FlickerComponent : public core::ecs::Component
{
	float_t scale = 1.f;
};

} // namespace blue_sky::ecs
