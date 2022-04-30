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

		if ( input->press( blue_sky::Input::Button::LEFT ) )
		{
			transform->transform.get_position() -= transform->transform.right() * 0.01f;
		}
		if ( input->press( blue_sky::Input::Button::RIGHT) )
		{
			transform->transform.get_position() += transform->transform.right() * 0.01f;
		}

		transform->transform.set_rotation( Quaternion( input->get_mouse_dx(), input->get_mouse_dy(), 0.f ) *  transform->transform.get_rotation() );
	}
}

} // namespace core::ecs
