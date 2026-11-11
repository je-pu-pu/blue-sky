#pragma once

#include <core/ecs/System.h>
#include <core/ecs/component/TransformComponent.h>
#include <blue_sky/ecs/component/StaticObjectComponent.h>

namespace blue_sky::ecs
{

/**
 * 周期的に動く物（TranslationComponent）を動かす（以前の TranslationObject::update）
 *
 * やり直したときの位置（SimpleObjectComponent::start_location）を中心に、位相を進めて位置を振り、剛体へ送る。
 * EntityManager::update() では動かさない（本編のシーンは呼ばない）。ActiveObjectManager::update が、物体の update の後に update_translations を呼ぶ
 */
class TranslationSystem : public core::ecs::System< TranslationComponent, core::ecs::TransformComponent >
{
public:
	TranslationSystem() = default;

	void update() override { }		///< 何もしない（上の説明のとおり）

	/// 位相を進めて位置を振る（毎フレーム）
	void update_translations( float_t delta_time );
};

} // namespace blue_sky::ecs
