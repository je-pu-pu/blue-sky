#pragma once

#include <core/ecs/System.h>
#include <core/ecs/EntityIdOrderedList.h>
#include <blue_sky/ecs/component/TargetMoverComponent.h>

#include <vector>

namespace blue_sky::ecs
{

/**
 * 物体を目標地点・目標の向きへ動かす（着いたら止めて、一度だけ知らせる）
 *
 * 以前は ActiveObjectManager::update が 3 つの unordered_map（ActiveObject* が鍵）を回していた。
 * 今は目標を Entity の部品（TargetLocationComponent・TargetDirectionComponent）に持ち、ここが Entity の番号の順（作った順）に回す。
 * 対象は TargetMoverComponent を持つ Entity（ActiveObject は加えるときに付ける）。
 *
 * EntityManager::update() では動かさない（本編のシーンは EntityManager::update を呼ばず、呼ぶシーンで二重に進めないため。
 * ObjectRenderDataSystem と同じ）。ActiveObjectManager::update が、物体の update とアニメーションの後に update_targets を呼ぶ
 * （以前と同じ所。物理を進める前）。
 */
class MoveToTargetSystem : public core::ecs::System< TargetMoverComponent >
{
private:
	/// 対象の Entity の番号（小さい順）。足すときは末尾に足し、外すときは印を付けて次に読むときにまとめて詰める
	/// （以前は対象が 1 つ増減するたびに全部を作り直して並べ替えていた。O(n log n)）
	core::ecs::EntityIdOrderedList< core::ecs::EntityId >	entity_id_list_;

	const std::vector< core::ecs::EntityId >& get_entity_ids() const { return entity_id_list_.get(); }

	/// id の Entity の部品（無い・消えた後なら nullptr）
	template< typename ComponentType >
	ComponentType* find_component( core::ecs::EntityId id ) const;

	void update_target_location( core::ecs::EntityId id, float_t delta_time );
	void update_target_direction( core::ecs::EntityId id );

protected:
	void on_remove_entity( core::ecs::EntityId id, ComponentTuple& ) override { entity_id_list_.erase( id ); }

public:
	MoveToTargetSystem() = default;

	void add_entity_component_if_all_components_ready( const core::ecs::Entity* entity ) override;

	void update() override { }						///< 何もしない（上の説明のとおり、呼ぶ所が決まっている）

	/**
	 * 目標地点へ動かすか、着いたら止めて知らせる。決まった向き・物体の方向へ向きを近づける（毎フレーム）
	 *
	 * 着いた物体は、目標地点に置いて速度を 0 にし、目標地点の部品を外してから notify_arrive_at_target_location を一度だけ呼ぶ
	 *
	 * @param delta_time このフレームの経過秒（着いたかの判定に使う）
	 */
	void update_targets( float_t delta_time );

	/**
	 * 目標地点を設定する（既にあれば置き換える）
	 *
	 * @return 対象の Entity（TargetMoverComponent を持つ）が無ければ false
	 */
	bool set_target_location( core::ecs::EntityId id, const core::Vector& location, float_t speed );

	/// 決まった向きを追わせる（物体の方向を追っていたらやめる）。対象が無ければ false
	bool set_target_direction( core::ecs::EntityId id, float_t degree, float_t speed );

	/// ほかの物体（target_id）の方向を追わせる（決まった向きを追っていたらやめる）。対象が無ければ false
	bool set_target_direction_entity( core::ecs::EntityId id, core::ecs::EntityId target_id, float_t speed );

	/// 全部の目標（地点・向き）を捨てる（ステージをやり直すとき）
	void clear_targets();

	/// 目標の位置に着いたか（残りが 0.1 未満か、このフレームで進める距離 speed × delta_time 以内）
	static bool is_target_location_reached( float_t distance, float_t speed, float_t delta_time );
};

} // namespace blue_sky::ecs
