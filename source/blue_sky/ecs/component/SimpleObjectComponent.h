#pragma once

#include <core/ecs/Component.h>
#include <core/type.h>

namespace blue_sky::ecs
{

/**
 * ECS だけの物体（ActiveObject を持たない物体）の、やり直しに使う状態と向き
 *
 * 以前は ActiveObject が持っていた（start_location_・start_rotation_・start_direction_degree_・direction_degree_）。
 * ObjectFactory が作るときに付け、EntityObject が読み書きする（やり直す・向きを変える・目標の向きを追う）。
 * 種類ごとの振る舞いは、ほかの部品（StaticObjectComponent・TranslationComponent など）の組み合わせで決まる
 */
struct SimpleObjectComponent : public core::ecs::Component
{
	string_t		kind;									///< 作った種類の名前（ステージ・スクリプトの名前。ログ・開発用の確かめに使う）

	core::Vector	start_location = core::Vector::Zero;	///< やり直したときの位置
	core::Vector	start_rotation = core::Vector::Zero;	///< やり直したときの回転（x・y・z 軸の度）
	float_t			start_direction_degree = 0.f;			///< やり直したときの向き（Y 軸の度）

	float_t			direction_degree = 0.f;					///< 今の向き（Y 軸の度。0 以上 360 以下）

	core::Vector	default_gravity = core::Vector::Zero;	///< やり直したときの重力（硬い物は地球の重力、硬くない物は 0。以前の GameObject::get_default_gravity）
	float_t			restart_mass = -1.f;					///< やり直したときに設定する質量（負なら変えない。以前の Goal::restart の set_mass( 100 )）
	float_t			restart_friction = -1.f;				///< やり直したときに設定する摩擦（負なら変えない。以前の Switch::restart の set_friction( 1.5 )）
	float_t			restart_angular_factor = -1.f;			///< やり直したときに設定する回転のしやすさ（負なら変えない。以前の Switch::restart の set_angular_factor( 0 )）
};

} // namespace blue_sky::ecs
