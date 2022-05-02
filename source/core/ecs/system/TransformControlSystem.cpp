#include "TransformControlSystem.h"
#include <blue_sky/GameMain.h> /// @todo core から blue_sky を参照しているのは変なので、直す
#include <blue_sky/Input.h> /// @todo core から blue_sky を参照しているのは変なので、直す

namespace core::ecs
{

void TransformControlSystem::update( ComponentTuple& component_tuple )
{
	using namespace blue_sky;

	const auto* input = GameMain::get_instance()->get_input();

	auto* transform = std::get< TransformComponent* >( component_tuple );
	const auto moving_speed = input->press( Input::Button::R2 ) ? 0.3f : 0.1f;

	if ( input->press( Input::Button::LEFT ) )
	{
		transform->transform.get_position() -= transform->transform.right() * moving_speed;
	}

	if ( input->press( Input::Button::RIGHT ) )
	{
		transform->transform.get_position() += transform->transform.right() * moving_speed;
	}

	if ( input->press( Input::Button::UP ) )
	{
		transform->transform.get_position() += transform->transform.forward() * moving_speed;
	}

	if ( input->press( Input::Button::DOWN ) )
	{
		transform->transform.get_position() -= transform->transform.forward() * moving_speed;
	}

	if ( input->press( Input::Button::L ) )
	{
		transform->transform.get_position() += transform->transform.up() * moving_speed;
	}

	if ( input->press( Input::Button::L2 ) )
	{
		transform->transform.get_position() -= transform->transform.up() * moving_speed;
	}

	const auto q = Quaternion( input->get_mouse_dx(), input->get_mouse_dy(), 0.f ) * transform->transform.get_rotation();
	// const auto zyx = q.get_euler_zyx();
	// q.set_euler_zyx( 0.f, zyx.y(), zyx.x() );
	transform->transform.set_rotation( q );
}

} // namespace core::ecs
