#pragma once

#include <core/ecs/Component.h>
#include <core/math/Vector.h>
#include <blue_sky/ai/EnemyPerception.h>

namespace blue_sky::ecs
{

/**
 * 敵の知覚コンポーネント（S2 知覚）。
 *
 * 視界錐のパラメータ（入力）と、そのフレームの知覚結果（出力）を持つ。
 * 判定ロジック本体は blue_sky::ai の純関数、対象探索と遮蔽(LOS)は知覚 System が行う。
 */
struct PerceptionComponent : public core::ecs::Component
{
	/// 視界錐パラメータ（距離・水平画角・上方許容角）
	ai::ViewCone cone;

	/// 目の高さ（Transform 原点からのオフセット, m）。視点位置＝position + (0, eye_height, 0)
	float eye_height = 1.4f;

	// --- 出力（知覚 System が毎フレーム書き込む） ---

	/// このフレーム、対象が見えているか（錐内かつ遮蔽なし）
	bool target_visible = false;

	/// 最後に対象を見た位置
	core::math::Vector last_seen_position;

	/// 一度でも対象を見たか（last_seen_position が有効か）
	bool has_last_seen = false;

}; // struct PerceptionComponent

} // namespace blue_sky::ecs
