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
	// タグのためデータは持たない。
}; // struct PerceivableComponent

} // namespace blue_sky::ecs
