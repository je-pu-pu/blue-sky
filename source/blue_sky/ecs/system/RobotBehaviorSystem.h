#pragma once

#include <core/ecs/System.h>
#include <core/ecs/component/TransformComponent.h>
#include <blue_sky/ecs/component/RobotBehaviorComponent.h>
#include <blue_sky/ecs/EntityObject.h>

#include <optional>
#include <vector>

namespace core::sound { class SoundManager; }
namespace core::physics { class PhysicsManager; }

namespace blue_sky
{
	namespace graphics { class GraphicsManager; }
}

namespace blue_sky::ecs
{

/**
 * 敵（RobotBehaviorComponent）を動かす（以前の Robot::update・restart・action・shutdown・on_collide_with( Player* )・on_collide_with_stone）
 *
 * 状態の移り変わりは RobotBehaviorComponent の説明のとおり。動かし方（速度・向き・アニメーション・音・テクスチャ）は以前の Robot と同じ値。
 *
 * 敵 AI のテストシーンの部品・System（PerceptionSystem・AlertSystem・PatrolSystem）は、巡回の地点を持つ PatrolComponent だけを使い、
 * 見え方・警戒・動かし方は使わない（ここにまとめて持つ）。理由 :
 * - 見え方 : 本編の敵は距離 3・10・20 m の 3 段で視野の広さが変わり（真後ろも見える近さがある）、目の高さは 1.5 m。
 *   PerceptionComponent の視界錐（距離 1 つ・水平の半画角・上の制限）では同じ判定にならない。遮るもののレイも本編の物理の世界
 *   （core::physics::PhysicsManager::ray_test_closest。相手と自分を通り抜ける・死んだ物と通り抜けられる物を通り抜ける）で飛ばす必要がある
 * - 警戒 : 本編は「見えたら 3 秒注目 → 追う → 10 秒で諦める」の時間で決まる状態で、AlertComponent の発見度（上がり下がりする値）とは振る舞いが違う
 * - 動かし方 : PatrolSystem は位置（Transform）を直接動かす。本編の敵は剛体の速度で動く（重力・壁に当たる）
 * 物理の世界は 1 つにした（決定 1。本編も PhysicsManager）が、見え方・警戒・動かし方の違いは残るので寄せていない。
 *
 * EntityManager::update() では動かさない（本編のシーンは呼ばない）。ActiveObjectManager::update が物体の update の直後に update_robots を呼ぶ
 */
class RobotBehaviorSystem : public core::ecs::System< RobotBehaviorComponent, core::ecs::TransformComponent >
{
public:
	using Mode = RobotBehaviorComponent::Mode;
	using Look = RobotBehaviorComponent::Look;

private:
	const core::physics::PhysicsManager*	physics_;			///< 見えるかのレイを飛ばす世界（nullptr ならレイを飛ばさず、遮るものは無いとみなす。テスト）
	graphics::GraphicsManager*			graphics_manager_;	///< モデル・テクスチャを引く先（nullptr ならテクスチャを設定しない。テスト）
	core::sound::SoundManager*			sound_manager_;		///< 音を鳴らす先（nullptr なら鳴らさない。テスト）
	std::vector< core::ecs::EntityId >	id_list_;			///< 作った順に回すための作業用の一覧（毎フレーム作り直す。中身は使い回す）

	RobotBehaviorComponent* find_robot( core::ecs::EntityId id ) const;

	/// 追う相手（いない・消えた・位置を持たないなら無い）。位置・速度・前・剛体は相手の部品から読む（以前はプレイヤーが実装する窓口 ChaseTarget）
	std::optional< EntityObject > find_target( const RobotBehaviorComponent& robot ) const;

	/// 今の見た目のテクスチャをモデルのシェーダーに設定する（以前の Robot::update の最後。モデルは敵どうしで共有しているので、最後に動かした敵の見た目になる。以前と同じ）
	void apply_look( core::ecs::Entity& entity, const RobotBehaviorComponent& robot ) const;

public:
	RobotBehaviorSystem( const core::physics::PhysicsManager* physics, graphics::GraphicsManager* graphics_manager, core::sound::SoundManager* sound_manager );

	void update() override { }		///< 何もしない（上の説明のとおり）

	/// 全部の敵を動かす（作った順 = Entity の番号の順。以前は物体の一覧の順）
	void update_robots( float_t delta_time, float_t scene_elapsed_time );

	/// 1 体の敵を動かす（敵でなければ何もしない）
	void update_robot( core::ecs::EntityId id, float_t delta_time, float_t scene_elapsed_time );

	/// 見た目ごとのテクスチャを名前で引いて持たせる（"robot"・"robot-warn"・"robot-error"・"robot-tender"。以前の Robot のコンストラクタ）
	void load_textures( core::ecs::EntityId id );

	/// 追う相手を決める（以前の Robot::set_player。相手は位置・向き・剛体の部品を持つ物体。プレイヤー）
	void set_target( core::ecs::EntityId id, core::ecs::EntityId target_id );

	/// やり直す（以前の Robot::restart の、ActiveObject::restart の後の所。EntityObject::restart が呼ぶ）
	static void restart( core::ecs::Entity& entity );

	/**
	 * 固有のアクション（以前の Robot::action。"rotation"・"patrol"・"float"）
	 *
	 * @return 敵のアクションとして受け付けたら true（敵でない・知らないアクションなら false）
	 */
	bool action( core::ecs::EntityId id, const string_t& action );

	/// 止める（以前の Robot::shutdown）
	void shutdown( core::ecs::EntityId id );

	/// 相手（プレイヤー）に当たった : 立つ・回る間なら止まる（以前の Robot::on_collide_with( Player* )）
	void on_touched_by_target( core::ecs::EntityId id );

	/**
	 * 石に当たった : 立つ・回る間に、こちらへ飛んでくる石なら止まる（以前の Robot::on_collide_with_stone。石は拾わない）
	 *
	 * @param is_stone_coming 石の速度がこちらへ近づく向きか（GameObject::is_moving_to）
	 */
	void on_hit_by_stone( core::ecs::EntityId id, bool is_stone_coming );

	/// 今の状態（敵でなければ無い）
	std::optional< Mode > get_mode( core::ecs::EntityId id ) const;

	/// 相手を目で見られるか（以前の Robot::calculate_target_visible。距離ごとの視野・遮るもの）
	bool is_target_visible( core::ecs::EntityId id ) const;
};

} // namespace blue_sky::ecs
