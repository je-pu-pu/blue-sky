#pragma once

#include <core/ecs/Component.h>
#include <core/type.h>

namespace blue_sky::ecs
{

/**
 * 少女（以前の Girl クラス）
 *
 * 立っている（STAND）か、浮いている（FLOAT）か。浮いている間は、浮く中心の位置でふわふわと上下する（GirlSystem）。
 * ステージ・スクリプトのアクション "float" で浮き、目標地点へ動かされて着くと、そこが浮く中心になる。やり直すと立つ。
 * 当たりでは何も起こさない（以前も Girl への当たりを受け取る物体は無かった）
 */
struct GirlComponent : public core::ecs::Component
{
	enum class Mode
	{
		STAND = 0,
		FLOAT,
	};

	Mode			mode = Mode::STAND;
	core::Vector	float_base_location = core::Vector::Zero;	///< 浮く中心（やり直すと始めの位置、目標地点に着くとその位置）
};

} // namespace blue_sky::ecs
