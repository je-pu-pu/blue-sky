#pragma once

#include <core/ecs/component/TransformComponent.h>
#include <blue_sky/ecs/component/PatrolComponent.h>
#include <blue_sky/ecs/component/AlertComponent.h>
#include <blue_sky/ecs/component/PerceptionComponent.h>
#include <core/ecs/System.h>

namespace core { class TimeManager; }
namespace core::nav { class NavMesh; }

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
 * navmesh を渡されていれば経路（Detour）をたどり、無ければ XZ 平面の直線移動。高さは維持する。
 * AlertSystem より後に走らせる。
 */
class PatrolSystem : public System< TransformComponent, PatrolComponent, AlertComponent, PerceptionComponent >
{
public:
	/// time_manager : 経過時間（作り手が渡す。以前は core::Service から取っていた）
	explicit PatrolSystem( const core::TimeManager* time_manager ) : time_manager_( time_manager ) { }

	void update() override;

	/// パトロール/追跡に使う navmesh を設定する（nullptr なら直線移動にフォールバック）。
	void set_navmesh( const core::nav::NavMesh* navmesh ) { navmesh_ = navmesh; }

private:
	/// 対象 1 つぶん（update() が対象ごとに呼ぶ。仮想関数ではないので、ループの中にインライン化できる）
	void update_entity( ComponentTuple& ) const;

private:
	const core::TimeManager* time_manager_;
	const core::nav::NavMesh* navmesh_ = nullptr;

}; // class PatrolSystem

} // namespace blue_sky::ecs
