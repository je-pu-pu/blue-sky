#pragma once

#include <core/ecs/Component.h>

namespace blue_sky::ecs
{

/**
 * 入力によって姿勢を更新する事をタグ付けするためのコンポーネント
 *
 */
class TransformControlComponent : public core::ecs::Component
{
public:
	float yaw	= 0.f; /// 姿勢の Y 軸における回転角度 ( radian ) 
	float pitch	= 0.f; /// 姿勢の X 軸における回転角度 ( radian ) 

}; // class TransformControlComponent

} // namespace blue_sky::ecs
