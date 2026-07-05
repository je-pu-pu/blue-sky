#pragma once

#include <core/ecs/component/TransformComponent.h>
#include <blue_sky/ecs/component/PatrolComponent.h>
#include <blue_sky/ecs/component/AlertComponent.h>
#include <blue_sky/ecs/component/PerceptionComponent.h>
#include <core/ecs/System.h>

namespace blue_sky::ecs
{

using core::ecs::System;
using core::ecs::TransformComponent;

/**
 * パトロール／捜索移動 System。
 *
 * 警戒状態に応じて敵の移動先を切り替える:
 *  - 無警戒: ウェイポイントを巡回（到達で待機→次へ）
 *  - 不審  : その場で最終視認方向を向く（移動しない）
 *  - 捜索/警報: 最終視認地点へ向かって移動する
 *
 * navmesh は未対応で、現状は XZ 平面の直線移動。高さは維持する。
 * AlertSystem より後に走らせる。
 */
class PatrolSystem : public System< TransformComponent, PatrolComponent, AlertComponent, PerceptionComponent >
{
public:
	void update( ComponentTuple& ) const override;

}; // class PatrolSystem

} // namespace blue_sky::ecs
