#pragma once

#include <core/ecs/component/TransformComponent.h>
#include <blue_sky/ecs/component/TransformControlComponent.h>
#include <core/ecs/System.h>

namespace core::input { class InputManager; }

namespace blue_sky::ecs
{

using core::ecs::System;
using core::ecs::TransformComponent;

/**
 * 入力によって姿勢を更新するシステム
 *
 */
class TransformControlSystem : public System< TransformComponent, TransformControlComponent >
{
public:

public:
	/// input : 入力（作り手が渡す。以前は core::Service から取っていた）
	explicit TransformControlSystem( const core::input::InputManager* input ) : input_( input ) { }
	~TransformControlSystem() = default;

	void update() override;

private:
	/// 対象 1 つぶん（update() が対象ごとに呼ぶ。仮想関数ではないので、ループの中にインライン化できる）
	void update_entity( ComponentTuple& ) const;

private:
	const core::input::InputManager* input_;

}; // class TransformControlSystem

} // namespace blue_sky::ecs
