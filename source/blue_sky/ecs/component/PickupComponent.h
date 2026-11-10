#pragma once

#include <core/ecs/Component.h>

namespace blue_sky::ecs
{

/// 拾う物の種類
enum class PickupKind
{
	MEDAL,		///< メダル（以前の Medal。拾うとメダルを持つ）
	ROCKET,		///< ロケット（以前の Rocket。拾うと持ち物が増える）
	UMBRELLA,	///< 傘（以前の Umbrella。拾っても何も増えない）
};

/**
 * 拾う物の印
 *
 * 以前は種類ごとのクラス（Medal・Rocket・Umbrella）の二重ディスパッチで、プレイヤーの on_collide_with( Medal* ) などが呼ばれていた。
 * 今は当たりの互換の層（ActiveObjectManager::on_collide_with_entity_object）が、これを持つ Entity に当たった ActiveObject の
 * GameObject::on_pick_up( kind ) を呼び、拾われたら（true が返ったら）この物体を死なせる
 */
struct PickupComponent : public core::ecs::Component
{
	PickupKind	kind = PickupKind::MEDAL;

	/// 拾われたら当たりの応答を消す（ロケット。やり直すと戻す）。以前の Rocket::on_collide_with( Player* )・Rocket::restart
	bool		disable_contact_on_pickup = false;
};

/**
 * 一定の速さで向きを変え続ける（Y 軸）
 *
 * 以前の Medal::update（1 秒に 90 度）。SpinSystem が毎フレーム回す
 */
struct SpinComponent : public core::ecs::Component
{
	float degree_per_second = 0.f;
};

} // namespace blue_sky::ecs
