#include "PlayerInputSystem.h"
#include "PlayerBehaviorSystem.h"
#include "PlayerInventorySystem.h"

#include <core/ecs/EntityManager.h>

namespace blue_sky::ecs
{

PlayerBehaviorSystem* PlayerInputSystem::find_behavior() const
{
	return get_entity_manager()->get_system< PlayerBehaviorSystem >();
}

PlayerInputComponent* PlayerInputSystem::find_input( core::ecs::EntityId id ) const
{
	core::ecs::Entity* entity = get_entity_manager()->find( id );

	return entity ? entity->get_component< PlayerInputComponent >() : nullptr;
}

PlayerInputSystem::Result PlayerInputSystem::update_player( core::ecs::EntityId id, const View& view )
{
	Result result;

	PlayerBehaviorSystem* behavior = find_behavior();
	PlayerInventorySystem* inventory = get_entity_manager()->get_system< PlayerInventorySystem >();
	const PlayerInputComponent* input = find_input( id );
	const PlayerBehaviorComponent* p = behavior ? behavior->find_player( id ) : nullptr;
	const PlayerInventoryComponent* items = inventory ? inventory->find_inventory( id ) : nullptr;

	if ( ! behavior || ! input || ! p || ! items )
	{
		return result;
	}

	using ItemType = PlayerBehaviorComponent::ItemType;
	using ActionMode = PlayerBehaviorComponent::ActionMode;

	// 上下の向きはカメラの上下の向きから決める
	behavior->set_pitch( id, -view.camera_rotate_x / view.mouse_rotation_speed );

	// 梯子では身を引き、見下ろしているときはマウスの縦の移動で身を乗り出す
	float_t eye_depth_add = 0.f;

	if ( p->is_on_ladder )
	{
		eye_depth_add = -0.05f;
	}
	else if ( view.camera_rotate_x > 0.f )
	{
		if ( view.is_mouse_look )
		{
			eye_depth_add = input->mouse_dy;
		}
	}

	behavior->add_eye_depth( id, eye_depth_add );

	if ( behavior->is_dead( id ) )
	{
		result.is_dead = true;
		result.restart_requested = input->a_pushed;

		return result;
	}

	bool is_moving = false;

	if ( ! p->is_ladder_step_only() )
	{
		if ( input->left )
		{
			behavior->side_step( id, -1.f );
			is_moving = true;
		}
		if ( input->right )
		{
			behavior->side_step( id, +1.f );
			is_moving = true;
		}
		if ( input->up )
		{
			behavior->step( id, +1.f );
			is_moving = true;
		}
		if ( input->down )
		{
			behavior->step( id, -1.f );
			is_moving = true;
		}
	}

	bool is_moving_on_ladder = false;

	if ( p->is_on_ladder )
	{
		if ( input->up )
		{
			behavior->ladder_step( id, +1.f );
			is_moving_on_ladder = true;
		}
		if ( input->down )
		{
			behavior->ladder_step( id, -1.f );
			is_moving_on_ladder = true;
		}
	}

	if ( ! is_moving )
	{
		behavior->stop( id );
	}

	if ( ! is_moving_on_ladder )
	{
		behavior->stop_ladder_step( id );
	}

	if ( input->a_pushed )
	{
		switch ( items->selected_item_type )
		{
			case ItemType::NONE: behavior->jump( id ); break;
			case ItemType::ROCKET: behavior->rocket( id, view.camera_front ); break;
			case ItemType::STONE: behavior->throw_stone( id, view.camera_front ); break;
			case ItemType::SCOPE:
			{
				behavior->switch_scope_mode( id );

				if ( p->action_mode == ActionMode::SCOPE )
				{
					result.scope_started = true;
				}
				break;
			}
			default: break;
		}
	}
	else if ( input->jump_pushed )
	{
		behavior->jump( id );
	}
	else if ( input->a || input->jump )
	{
		behavior->clamber( id );
	}
	else
	{
		behavior->stop_clamber( id );
	}

	if ( p->action_mode == ActionMode::SCOPE )
	{
		result.scope_wheel = input->wheel;
	}
	else
	{
		if ( input->wheel > 0 )
		{
			inventory->select_next_item( id );
		}
		else if ( input->wheel < 0 )
		{
			inventory->select_prev_item( id );
		}

		result.reset_fov = true;
	}

	behavior->add_direction_degree( id, input->mouse_dx * view.mouse_rotation_speed * view.rotation_speed_rate + input->hmd_delta_yaw_degree );

	return result;
}

} // namespace blue_sky::ecs
