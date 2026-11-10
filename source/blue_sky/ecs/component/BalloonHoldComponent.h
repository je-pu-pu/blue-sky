#pragma once

#include <core/ecs/Component.h>
#include <core/ecs/type.h>
#include <core/type.h>

#include <optional>

namespace blue_sky::ecs
{

/**
 * 風船を持つ物体（プレイヤー）の窓口（BalloonHoldSystem が、持たれている風船を手元に合わせるときに呼ぶ）
 *
 * プレイヤーはまだ ActiveObject なので、プレイヤーがこれを実装する（前・右の向きと、風船の動作の状態はプレイヤーが持つ）。
 * 段階 6 でプレイヤーを ECS にしたら、部品から読む形に替える。テストでは偽物を実装する
 */
class BalloonHolder
{
public:
	/// 今持っている風船が balloon か（後から別の風船をつかむと、前の風船は持たれていない）
	virtual bool is_holding_balloon( core::ecs::EntityId balloon ) const = 0;

	/// 風船の動作の途中か（終わると持っていた風船は割れる）
	virtual bool is_in_balloon_action() const = 0;

	/// 風船の動作が終わりかけか（終わりかけは風船が点滅する）
	virtual bool is_balloon_action_pre_finish() const = 0;

	/// 持っている風船を置く位置（手元）
	virtual core::Vector get_balloon_hold_location() const = 0;

protected:
	~BalloonHolder() = default;	///< これを通して壊さない（持ち主は ActiveObjectManager）
};

/**
 * プレイヤーがつかまる風船（以前の Balloon クラス）
 *
 * 持たれていない間は、始めの位置でふわふわと上下しながら回る。プレイヤーが触れるとつかまれ、手元に付いて行く。
 * プレイヤーが別の風船をつかむか、風船の動作が終わると割れる（死んで (0, -100, 0) へ退く）。石が当たっても割れる。
 * 振る舞いは BalloonHoldSystem。
 *
 * ECS のテストシーン（MusicGamePrototypeScene）の BalloonComponent とは別のもの（あちらは ECS の物理の世界で、触れた物に上向きの力を与える）
 */
struct BalloonHoldComponent : public core::ecs::Component
{
	std::optional< core::ecs::EntityId >	holder_id;			///< つかんでいる物体（つかまれていなければ無い。やり直すと無くなる）
	BalloonHolder*							holder = nullptr;	///< つかんでいる物体の窓口（持ち主ではない。holder_id の Entity が生きている間だけ使う）

	static constexpr float_t FlickerScale = 0.5f;			///< ゆらぎの大きさ（以前の Balloon::update。物体のゆらぎの大きさの設定は使わない）
	static constexpr float_t SpinDegreePerFrame = 1.f;		///< 持たれていない間に 1 フレームで回る度（以前の Balloon::update）
	static constexpr float_t BlinkCount = 6.f;				///< 動作が終わりかけのとき、1 秒に点滅する回数

	bool is_held() const { return holder_id.has_value(); }
};

} // namespace blue_sky::ecs
