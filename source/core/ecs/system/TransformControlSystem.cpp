#include "TransformControlSystem.h"
#include <blue_sky/GameMain.h> /// @todo core から blue_sky を参照しているのは変なので、直す
#include <blue_sky/Input.h> /// @todo core から blue_sky を参照しているのは変なので、直す

namespace core::ecs
{

void TransformControlSystem::update()
{
	const auto* input = blue_sky::GameMain::get_instance()->get_input();

	for ( auto& i : get_component_list() )
	{
		auto* transform = std::get< TransformComponent* >( i.second );
		const auto moving_speed = 0.01f;

		if ( input->press( blue_sky::Input::Button::LEFT ) )
		{
			transform->transform.get_position() -= transform->transform.right() * moving_speed;
		}

		if ( input->press( blue_sky::Input::Button::RIGHT ) )
		{
			transform->transform.get_position() += transform->transform.right() * moving_speed;
		}

		if ( input->press( blue_sky::Input::Button::UP ) )
		{
			transform->transform.get_position() += transform->transform.forward() * moving_speed;
		}

		if ( input->press( blue_sky::Input::Button::DOWN ) )
		{
			transform->transform.get_position() -= transform->transform.forward() * moving_speed;
		}

		if ( input->press( blue_sky::Input::Button::L ) )
		{
			transform->transform.get_position() += transform->transform.up() * moving_speed;
		}

		if ( input->press( blue_sky::Input::Button::L2 ) )
		{
			transform->transform.get_position() -= transform->transform.up() * moving_speed;
		}

		const auto q = Quaternion( input->get_mouse_dx(), input->get_mouse_dy(), 0.f ) * transform->transform.get_rotation();
		// const auto zyx = q.get_euler_zyx();
		// q.set_euler_zyx( 0.f, zyx.y(), zyx.x() );
		transform->transform.set_rotation( q );
		
		
	}
}

} // namespace core::ecs
