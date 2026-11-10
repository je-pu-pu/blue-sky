#pragma once

#include <core/ecs/System.h>
#include <core/ecs/component/TransformComponent.h>
#include <blue_sky/ecs/component/GirlComponent.h>

namespace blue_sky::ecs
{

/**
 * 少女（GirlComponent）を動かす（以前の Girl::update・action）
 *
 * 浮いている間は、毎フレーム速度を 0 にしてから、浮く中心にゆらぎの高さのずれを足した所へ上下の速度を向ける。
 * EntityManager::update() では動かさない（本編のシーンは呼ばない）。ActiveObjectManager::update が物体の update の直後に update_girls を呼ぶ
 */
class GirlSystem : public core::ecs::System< GirlComponent, core::ecs::TransformComponent >
{
public:
	GirlSystem() = default;

	void update() override { }		///< 何もしない（上の説明のとおり）

	/// 浮いている少女を動かす（毎フレーム。少女どうしは互いに影響しないので、回す順は決めない）
	void update_girls( float_t scene_elapsed_time );

	/**
	 * 固有のアクション（以前の Girl::action。"float" で浮く）
	 *
	 * @return 少女のアクションとして受け付けたら true（少女でない・知らないアクションなら false）
	 */
	bool action( core::ecs::EntityId id, const string_t& action );
};

} // namespace blue_sky::ecs
