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
 * 以前の Medal::update（1 秒に 90 度）・Stone::update（1 フレームに 1 度。経った時間によらない）。SpinSystem が毎フレーム回す
 */
struct SpinComponent : public core::ecs::Component
{
	float degree_per_second = 0.f;
	float degree_per_frame = 0.f;
};

/**
 * 投げられる物（石）の印
 *
 * 以前の Stone クラスの代わり。当たりの互換の層（ActiveObjectManager::on_collide_with_entity_object）が、
 * これを持つ Entity に当たった物体の GameObject::on_collide_with_stone に石の窓口（GameObjectRef）を渡し、
 * 拾われたら（true なら）石を死なせる。スイッチ（ECS だけの物体）との当たりは on_collide_entity_objects。
 * やり直すと（投げるときも）、薄い壁を貫通しないように連続衝突判定を設定する（以前の Stone::restart）
 */
struct ThrowableComponent : public core::ecs::Component
{
	float ccd_swept_sphere_radius = 0.2f;
	float ccd_motion_threshold = 0.2f;
};

} // namespace blue_sky::ecs
