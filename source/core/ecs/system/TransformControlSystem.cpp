#include "TransformControlSystem.h"
#include <core/Service.h>
#include <core/input/InputManager.h>

#include <common/math.h>
#include <iostream>

namespace core::ecs
{

void TransformControlSystem::update( ComponentTuple& component_tuple ) const
{
	using Button = core::input::Button;

	const auto* input = core::get_input_manager();

	auto* transform = std::get< TransformComponent* >( component_tuple );
	auto* transform_control = std::get< TransformControlComponent* >( component_tuple );

	const auto moving_speed = input->press( Button::R2 ) ? 0.3f : 0.1f;

	if ( input->press( Button::LEFT ) )
	{
		transform->transform.get_position() -= transform->transform.right() * moving_speed;
	}

	if ( input->press( Button::RIGHT ) )
	{
		transform->transform.get_position() += transform->transform.right() * moving_speed;
	}

	if ( input->press( Button::UP ) )
	{
		transform->transform.get_position() += transform->transform.forward() * moving_speed;
	}

	if ( input->press( Button::DOWN ) )
	{
		transform->transform.get_position() -= transform->transform.forward() * moving_speed;
	}

	if ( input->press( Button::L ) )
	{
		transform->transform.get_position() += transform->transform.up() * moving_speed;
	}

	if ( input->press( Button::L2 ) )
	{
		transform->transform.get_position() -= transform->transform.up() * moving_speed;
	}

	transform_control->yaw += input->get_mouse_dx();
	transform_control->pitch += input->get_mouse_dy();

	if ( transform_control->yaw < ::math::degree_to_radian( -180.f ) )
	{
		transform_control->yaw += ::math::degree_to_radian( 360.f );
	}

	if ( transform_control->yaw > ::math::degree_to_radian( 180.f ) )
	{
		transform_control->yaw -= ::math::degree_to_radian( 360.f );
	}

	transform_control->pitch = ::math::clamp( transform_control->pitch, ::math::degree_to_radian( -90.f ), ::math::degree_to_radian( 90.f ) );

	transform->transform.get_rotation().set_yaw_pitch_roll( transform_control->yaw, transform_control->pitch, 0 );
}

} // namespace core::ecs
