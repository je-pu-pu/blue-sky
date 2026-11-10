#pragma once

#include <core/ecs/Component.h>
#include <core/animation/AnimationPlayer.h>

#include <memory>

namespace blue_sky::ecs
{

/**
 * スキニングアニメーションの再生（ボーン行列の計算）
 *
 * アニメーションを持つモデルの物体だけが持つ。以前は ActiveObject が unique_ptr< AnimationPlayer > を持っていた。
 * 毎フレームの進め方は ObjectRenderDataSystem::update_animations、ボーン行列の書き込みは ObjectRenderDataSystem::update_render_data
 */
struct AnimationComponent : public core::ecs::Component
{
	std::unique_ptr< core::AnimationPlayer > player;
};

} // namespace blue_sky::ecs
