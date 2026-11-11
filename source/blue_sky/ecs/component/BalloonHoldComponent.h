#pragma once

#include <core/ecs/Component.h>
#include <core/ecs/type.h>
#include <core/type.h>

#include <optional>

namespace blue_sky::ecs
{

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
	/// つかんでいるプレイヤー（つかまれていなければ無い。やり直すと無くなる）。
	/// 持っている風船・風船の動作の状態・手元の位置は、プレイヤーの部品（PlayerBehaviorComponent と位置・向き）から読む
	/// （以前はプレイヤーが実装する窓口 BalloonHolder を持っていた。段階 6 でプレイヤーを ECS だけの物体にしたので部品を読む）
	std::optional< core::ecs::EntityId >	holder_id;

	static constexpr float_t FlickerScale = 0.5f;			///< ゆらぎの大きさ（以前の Balloon::update。物体のゆらぎの大きさの設定は使わない）
	static constexpr float_t SpinDegreePerFrame = 1.f;		///< 持たれていない間に 1 フレームで回る度（以前の Balloon::update）
	static constexpr float_t BlinkCount = 6.f;				///< 動作が終わりかけのとき、1 秒に点滅する回数

	bool is_held() const { return holder_id.has_value(); }
};

} // namespace blue_sky::ecs
