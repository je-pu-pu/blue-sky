#pragma once

#include <core/ecs/Component.h>

namespace blue_sky::ecs
{

/**
 * 「敵に知覚され得る対象」であることを示すタグ コンポーネント。
 *
 * 敵AI（知覚 System）は PlayerComponent 等の具体型を一切知らず、この
 * タグ＋TransformComponent を持つ実体だけを視界判定の対象にする。
 * これにより S2 知覚 / S3 敵AI を「プレイヤー/敵の型を知らない中立サービス」に保ち、
 * 上位（プレイヤー動詞）への逆依存＝循環を防ぐ（システム階層スタックの中核原則）。
 */
struct PerceivableComponent : public core::ecs::Component
{
	/// この対象が今フレーム音を発しているか（走行・道具使用など）。敵は聴覚(hears_noise)で拾う。
	bool  emitting_noise = false;

	/// 音の大きさ倍率（実効可聴半径 = 敵の hearing_radius × これ）。走行=大, 忍び足=小, 花火=特大。
	float noise_loudness = 1.f;

}; // struct PerceivableComponent

} // namespace blue_sky::ecs
