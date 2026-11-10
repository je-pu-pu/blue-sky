#pragma once

#include <core/ecs/System.h>
#include <core/ecs/component/TransformComponent.h>
#include <blue_sky/ecs/component/BalloonHoldComponent.h>

#include <vector>

namespace core::sound { class SoundManager; }

namespace blue_sky::ecs
{

/**
 * プレイヤーがつかまる風船（BalloonHoldComponent）を動かす（以前の Balloon::update・kill・on_collide_with_stone）
 *
 * - つかまれていない : 始めの位置でふわふわと上下し、1 フレームに 1 度回る
 * - つかまれている : つかんだ物体の手元に置き、向きを 0 にする。つかんだ物体が別の風船を持った・風船の動作が終わったら割れる（死ぬ）。
 *   動作が終わりかけの間は点滅する（描画の部品の is_hidden）
 *
 * 死んだ風船も以前と同じく動かし続ける（割れた風船は、つかまれていなければ始めの位置へ戻る。描かず、当たりも起こさない）。
 *
 * EntityManager::update() では動かさない（本編のシーンは呼ばない）。ActiveObjectManager::update が物体の update の直後に update_balloons を、
 * GamePlayScene が物理を進めた後にプレイヤーの持っている風船だけ update_balloon を呼ぶ（以前と同じ所。手元が 1 フレーム遅れないように）
 */
class BalloonHoldSystem : public core::ecs::System< BalloonHoldComponent, core::ecs::TransformComponent >
{
private:
	core::sound::SoundManager*			sound_manager_;		///< 割れる音を鳴らす先（nullptr なら鳴らさない。テスト）
	std::vector< core::ecs::EntityId >	id_list_;			///< 作った順に回すための作業用の一覧（毎フレーム作り直す。中身は使い回す）

public:
	explicit BalloonHoldSystem( core::sound::SoundManager* sound_manager ) : sound_manager_( sound_manager ) { }

	void update() override { }		///< 何もしない（上の説明のとおり、呼ぶ所が決まっている）

	/// 全部の風船を動かす（作った順 = Entity の番号の順。以前は物体の一覧の順）
	void update_balloons( float_t scene_elapsed_time );

	/// 1 つの風船を動かす（風船でなければ何もしない）
	void update_balloon( core::ecs::EntityId id, float_t scene_elapsed_time );

	/**
	 * 風船をつかませる
	 *
	 * @return つかませたら true。風船でない・既にほかの物体につかまれているなら false
	 */
	bool grab( core::ecs::EntityId balloon, core::ecs::EntityId holder_id, BalloonHolder* holder );

	/// 石が当たった : 割れて音を鳴らす（以前の Balloon::on_collide_with_stone。石は拾わない）
	void burst( core::ecs::EntityId id );

	/// 風船か
	bool is_balloon( core::ecs::EntityId id ) const;
};

} // namespace blue_sky::ecs
