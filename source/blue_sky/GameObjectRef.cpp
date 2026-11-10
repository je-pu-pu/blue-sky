#include "GameObjectRef.h"

#include <blue_sky/ConstantBuffers.h>
#include <blue_sky/DebugLog.h>
#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/graphics/Model.h>

#include <blue_sky/ecs/EntityObject.h>
#include <blue_sky/ecs/ObjectBody.h>
#include <blue_sky/ecs/component/AnimationComponent.h>
#include <blue_sky/ecs/component/ObjectRenderComponent.h>
#include <blue_sky/ecs/component/SimpleObjectComponent.h>
#include <blue_sky/ecs/system/MoveToTargetSystem.h>
#include <blue_sky/ecs/system/SwitchSystem.h>
#include <blue_sky/ecs/component/SwitchComponent.h>
#include <blue_sky/ecs/component/FlickerComponent.h>
#include <blue_sky/ecs/system/GirlSystem.h>
#include <blue_sky/ecs/system/RobotBehaviorSystem.h>

#include <core/animation/AnimationPlayer.h>
#include <core/ecs/EntityManager.h>

#include <sstream>
#include <string>

namespace blue_sky
{

GameObjectRef::GameObjectRef( EntityManager* entity_manager, graphics::GraphicsManager* graphics_manager, EntityId id )
	: entity_manager_( entity_manager )
	, graphics_manager_( graphics_manager )
	, id_( id )
{

}

core::ecs::Entity* GameObjectRef::find_entity() const
{
	return entity_manager_ ? entity_manager_->find( id_ ) : nullptr;
}

bool GameObjectRef::is_alive() const
{
	return find_entity() != nullptr;
}

void GameObjectRef::log_unsupported( const char* operation ) const
{
	const core::ecs::Entity* entity = find_entity();
	const auto* state = entity ? entity->get_component< ecs::SimpleObjectComponent >() : nullptr;

	debug_log( string_t( "[Object] この物体では " ) + operation + " はできません（種類 : " + ( state ? state->kind : string_t( "不明" ) ) + "）" );
}

btRigidBody* GameObjectRef::get_rigid_body() const
{
	return ecs::EntityObject( find_entity() ).get_rigid_body();
}

core::Vector GameObjectRef::get_location() const
{
	ecs::EntityObject object( find_entity() );

	return object.is_valid() ? object.get_location() : core::Vector::Zero;
}

void GameObjectRef::set_location( const core::Vector& location )
{
	ecs::EntityObject object( find_entity() );

	if ( object.is_valid() )
	{
		object.set_location( location );
	}
}

float_t GameObjectRef::get_direction_degree() const
{
	ecs::EntityObject object( find_entity() );

	return object.is_valid() ? object.get_direction_degree() : 0.f;
}

void GameObjectRef::set_direction_degree( float_t degree )
{
	ecs::EntityObject object( find_entity() );

	if ( object.is_valid() )
	{
		object.set_direction_degree( degree );
	}
}

core::Vector GameObjectRef::get_velocity() const
{
	ecs::EntityObject object( find_entity() );

	return object.is_valid() ? object.get_velocity() : core::Vector::Zero;
}

bool GameObjectRef::is_moving_to( const core::Vector& location ) const
{
	return ecs::object_body::is_moving_to( get_location(), get_velocity(), location );
}

void GameObjectRef::set_velocity( const core::Vector& velocity )
{
	ecs::EntityObject object( find_entity() );

	if ( object.is_valid() )
	{
		object.set_velocity( velocity );
	}
}

void GameObjectRef::set_start_location( float_t x, float_t y, float_t z )
{
	ecs::EntityObject object( find_entity() );

	if ( object.is_valid() )
	{
		object.set_start_location( core::Vector( x, y, z ) );
	}
}

void GameObjectRef::set_start_rotation( float_t x, float_t y, float_t z )
{
	ecs::EntityObject object( find_entity() );

	if ( object.is_valid() )
	{
		object.set_start_rotation( x, y, z );
	}
}

void GameObjectRef::set_start_direction_degree( float_t degree )
{
	ecs::EntityObject object( find_entity() );

	if ( object.is_valid() )
	{
		object.set_start_direction_degree( degree );
	}
}

namespace
{

/// ECS だけの物体の剛体（無ければ nullptr）
btRigidBody* find_rigid_body( core::ecs::Entity* entity )
{
	return ecs::EntityObject( entity ).get_rigid_body();
}

} // namespace

void GameObjectRef::set_mass( float_t mass )
{
	if ( btRigidBody* body = find_rigid_body( find_entity() ) )
	{
		ecs::object_body::set_mass( * body, mass );
	}
}

void GameObjectRef::set_gravity( const core::Vector& gravity )
{
	if ( btRigidBody* body = find_rigid_body( find_entity() ) )
	{
		ecs::object_body::set_gravity( * body, gravity );
	}
}

void GameObjectRef::set_friction( float_t friction )
{
	if ( btRigidBody* body = find_rigid_body( find_entity() ) )
	{
		ecs::object_body::set_friction( * body, friction );
	}
}

void GameObjectRef::set_angular_factor( const core::Vector& factor )
{
	if ( btRigidBody* body = find_rigid_body( find_entity() ) )
	{
		ecs::object_body::set_angular_factor( * body, factor );
	}
}

void GameObjectRef::set_kinematic( bool is_kinematic )
{
	if ( btRigidBody* body = find_rigid_body( find_entity() ) )
	{
		ecs::object_body::set_kinematic( * body, is_kinematic );
	}
}

void GameObjectRef::set_no_contact_response( bool is_no_contact_response )
{
	if ( btRigidBody* body = find_rigid_body( find_entity() ) )
	{
		ecs::object_body::set_no_contact_response( * body, is_no_contact_response );
	}
}

void GameObjectRef::set_flicker_scale( float_t scale )
{
	// ECS だけの物体は、ゆらぎの大きさの部品を持つもの（少女・敵）だけ
	core::ecs::Entity* entity = find_entity();

	if ( auto* flicker = entity ? entity->get_component< ecs::FlickerComponent >() : nullptr )
	{
		flicker->scale = scale;
		return;
	}

	log_unsupported( "ゆらぎの大きさの設定" );
}

void GameObjectRef::play_animation( const char_t* name, bool force, bool loop )
{
	core::ecs::Entity* entity = find_entity();

	if ( auto* animation = entity ? entity->get_component< ecs::AnimationComponent >() : nullptr )
	{
		animation->player->play( name, force, loop );
	}
}

void GameObjectRef::set_animation_speed( float_t speed )
{
	core::ecs::Entity* entity = find_entity();

	if ( auto* animation = entity ? entity->get_component< ecs::AnimationComponent >() : nullptr )
	{
		animation->player->set_speed( speed );
	}
}

void GameObjectRef::action( const string_t& action )
{
	core::ecs::Entity* entity = find_entity();

	if ( ! entity )
	{
		return;
	}

	// スイッチ（以前の BaseSwitch::action。この 3 つだけを受け付け、イベントは呼ばない）
	if ( entity->get_component< ecs::SwitchComponent >() )
	{
		auto* system = entity_manager_->get_system< ecs::SwitchSystem >();

		if ( system && action == "turn-on" )
		{
			system->turn_on( id_ );
		}
		else if ( system && action == "turn-off" )
		{
			system->turn_off( id_ );
		}
		else if ( system && action == "break" )
		{
			system->do_break( id_ );
		}
		else
		{
			log_unsupported( ( "アクション \"" + action + "\"" ).c_str() );
		}

		return;
	}

	// 種類ごとのアクション（以前の Girl::action の "float"、Robot::action の "rotation"・"patrol"・"float"）
	if ( auto* system = entity_manager_->get_system< ecs::GirlSystem >(); system && system->action( id_, action ) )
	{
		return;
	}

	if ( auto* system = entity_manager_->get_system< ecs::RobotBehaviorSystem >(); system && system->action( id_, action ) )
	{
		return;
	}

	// 以前の ActiveObject::action と同じ中身
	auto* animation = entity->get_component< ecs::AnimationComponent >();

	if ( action == "break_animation 1" && animation )
	{
		animation->player->set_broken( true );
		return;
	}

	if ( action == "break_animation 0" && animation )
	{
		animation->player->set_broken( false );
		return;
	}

	std::stringstream ss;
	ss << action;

	string_t command;
	ss >> command;

	if ( command == "set_visible" )
	{
		bool model = true, line = true;
		ss >> model >> line;

		ecs::EntityObject( entity ).set_visible( model, line );
		return;
	}

	log_unsupported( ( "アクション \"" + action + "\"" ).c_str() );
}

void GameObjectRef::kill()
{
	if ( core::ecs::Entity* entity = find_entity() )
	{
		ecs::EntityObject( entity ).kill();
	}
}

void GameObjectRef::restart()
{
	ecs::EntityObject object( find_entity() );

	if ( object.is_valid() )
	{
		object.restart();
	}
}

GameObjectRef::Model* GameObjectRef::get_model() const
{
	core::ecs::Entity* entity = find_entity();
	const auto* render = entity ? entity->get_component< ecs::ObjectRenderComponent >() : nullptr;

	return render && graphics_manager_ ? graphics_manager_->resolve_model( render->model ) : nullptr;
}

void GameObjectRef::set_model( Model* model )
{
	core::ecs::Entity* entity = find_entity();
	auto* render = entity ? entity->get_component< ecs::ObjectRenderComponent >() : nullptr;

	if ( ! render || ! graphics_manager_ )
	{
		log_unsupported( "モデルの設定" );
		return;
	}

	// 以前の ActiveObject::set_model・setup_animation_player と同じ中身
	render->model = graphics_manager_->get_model_handle( model );

	if ( model && render->model.is_null() )
	{
		// GraphicsManager が管理していないモデルはハンドルにできず、描けない
		debug_log( "[Object] GraphicsManager が管理していないモデルは使えません（描きません）" );
	}

	if ( model && model->get_skinning_animation_set() && ! entity->get_component< ecs::AnimationComponent >() )
	{
		auto* animation = entity->add_component< ecs::AnimationComponent >();
		animation->player = std::make_unique< core::AnimationPlayer >( model->get_skinning_animation_set() );
		render->bone_constant_buffer = graphics_manager_->create_constant_buffer( sizeof( core::BoneConstantBufferData ), core::BoneConstantBuffer::SLOT, core::graphics::ConstantBufferUsage::DYNAMIC );	// 毎フレーム書き換える
	}
}

bool GameObjectRef::make_model_unique()
{
	Model* model = get_model();

	if ( ! model || ! graphics_manager_ )
	{
		return false;
	}

	set_model( graphics_manager_->clone_model_with_shaders( model ) );

	return true;
}

void GameObjectRef::add_event_handler( const char_t* name, const EventHandler& handler )
{
	// スイッチ（以前の BaseSwitch::add_event_handler）
	if ( auto* system = entity_manager_ ? entity_manager_->get_system< ecs::SwitchSystem >() : nullptr; system && system->add_event_handler( id_, name, handler ) )
	{
		return;
	}

	log_unsupported( ( string_t( "イベント \"" ) + name + "\" の受け取り" ).c_str() );
}

void GameObjectRef::set_target_location( const core::Vector& location, float_t speed )
{
	auto* system = entity_manager_ ? entity_manager_->get_system< ecs::MoveToTargetSystem >() : nullptr;

	if ( ! system || ! is_alive() || ! system->set_target_location( id_, location, speed ) )
	{
		debug_log( "[Object] 目標地点を設定できません（物体の Entity が無い。片付けた後）" );
		return;
	}

	const core::Vector from = get_location();
	debug_log( "[Object] 目標地点へ動かします ( " + std::to_string( from.x() ) + ", " + std::to_string( from.y() ) + ", " + std::to_string( from.z() ) + " ) → ( "
		+ std::to_string( location.x() ) + ", " + std::to_string( location.y() ) + ", " + std::to_string( location.z() ) + " ) 速さ " + std::to_string( speed ) );
}

void GameObjectRef::set_target_direction( float_t degree, float_t speed )
{
	auto* system = entity_manager_ ? entity_manager_->get_system< ecs::MoveToTargetSystem >() : nullptr;

	if ( ! system || ! is_alive() || ! system->set_target_direction( id_, degree, speed ) )
	{
		debug_log( "[Object] 目標の向きを設定できません（物体の Entity が無い）" );
	}
}

void GameObjectRef::set_target_direction_to( const GameObjectRef& target, float_t speed )
{
	auto* system = entity_manager_ ? entity_manager_->get_system< ecs::MoveToTargetSystem >() : nullptr;

	if ( ! system || ! is_alive() || ! target.is_alive() || ! system->set_target_direction_entity( id_, target.get_entity_id(), speed ) )
	{
		debug_log( "[Object] 向く先の物体を設定できません（物体の Entity が無い）" );
	}
}

} // namespace blue_sky
