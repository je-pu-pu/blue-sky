#pragma once

#include <core/ecs/Component.h>
#include <core/type.h>

namespace blue_sky::ecs
{

/**
 * 動かない物（壁・建物・箱などステージに置く物）の印
 *
 * 以前の StaticObject クラスの代わり。当たりでは、ActiveObject（プレイヤーなど）がこれを持つ Entity に当たると
 * GameObject::on_collide_with_static_object が呼ばれる（以前の on_collide_with( StaticObject* )。プレイヤーはロケットを止める）
 */
struct StaticObjectComponent : public core::ecs::Component
{

};

/**
 * 周期的に動く物（動く床など。以前の TranslationObject）
 *
 * やり直したときの位置を中心に、sin・cos で位置を振る。TranslationSystem が毎フレーム動かす
 */
struct TranslationComponent : public core::ecs::Component
{
	float_t width = 0.f;		///< x の振れ幅（cos）
	float_t height = 0.f;		///< y の振れ幅（sin）
	float_t depth = 0.f;		///< z の振れ幅（cos）
	float_t speed = 0.f;		///< 1 秒に進む位相（ラジアン）
	float_t angle = 0.f;		///< 今の位相（ラジアン。やり直しても戻さない。以前と同じ）
};

} // namespace blue_sky::ecs
