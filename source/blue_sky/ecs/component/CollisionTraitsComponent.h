#pragma once

#include <core/ecs/Component.h>

namespace blue_sky::ecs
{

/**
 * 物体の当たりの性質（すり抜ける・硬い・壁になる・足場になる）と、死んでいるか
 *
 * 当たりの振り分け（CollisionEventSystem）・レイの判定（ClosestNotMe）・プレイヤーの壁の判定が、
 * 剛体の Entity の番号のタグからこれを引く。以前は userPointer の ActiveObject* の仮想関数
 * （is_ghost・is_hard・is_block・is_safe_footing・is_dead）を呼んでいた。
 *
 * 性質は種類ごとに決まっている（派生クラスの仮想関数はどれも定数を返す）ので、ActiveObject が加えられるとき（attach_entity）に一度だけ写す。
 * 死んでいるかは ActiveObject::kill・restart が書く（ActiveObject の is_dead_ と同じ値を保つ）
 */
struct CollisionTraitsComponent : public core::ecs::Component
{
	bool is_ghost = false;			///< 通り抜けられる（レイが当たらない）
	bool is_hard = true;			///< ぶつかると反発する硬い物（硬くない物は、足場を探すレイで足場に含めるときだけ当たる）
	bool is_block = true;			///< 壁・障害物になる（プレイヤーの前の壁の判定）
	bool is_safe_footing = false;	///< 着地しても死なない足場（風船・ゴール）

	bool is_dead = false;			///< 死んでいる（当たりを起こさない・レイが通り抜ける）
};

} // namespace blue_sky::ecs
