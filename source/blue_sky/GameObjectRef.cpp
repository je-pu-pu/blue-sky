#include "GameObjectRef.h"

#include <GameObject/ActiveObject.h>

#include <blue_sky/ConstantBuffers.h>
#include <blue_sky/DebugLog.h>
#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/graphics/Model.h>

#include <blue_sky/ecs/EntityObject.h>
#include <blue_sky/ecs/ObjectBody.h>
#include <blue_sky/ecs/component/ActiveObjectLinkComponent.h>
#include <blue_sky/ecs/component/AnimationComponent.h>
#include <blue_sky/ecs/component/ObjectRenderComponent.h>
#include <blue_sky/ecs/component/SimpleObjectComponent.h>
#include <blue_sky/ecs/system/MoveToTargetSystem.h>

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

ActiveObject* GameObjectRef::get_active_object() const
{
	core::ecs::Entity* entity = find_entity();
	const auto* link = entity ? entity->get_component< ecs::ActiveObjectLinkComponent >() : nullptr;

	return link ? link->object : nullptr;
}

void GameObjectRef::log_unsupported( const char* operation ) const
{
	const core::ecs::Entity* entity = find_entity();
	const auto* state = entity ? entity->get_component< ecs::SimpleObjectComponent >() : nullptr;

	debug_log( string_t( "[Object] この物体では " ) + operation + " はできません（種類 : " + ( state ? state->kind : string_t( "不明" ) ) + "）" );
}

btRigidBody* GameObjectRef::get_rigid_body() const
{
	if ( ActiveObject* o = get_active_object() )
	{
		return o->get_rigid_body();
	}

	return ecs::EntityObject( find_entity() ).get_rigid_body();
}

core::Vector GameObjectRef::get_location() const
{
	if ( const ActiveObject* o = get_active_object() )
	{
		return o->get_location();
	}

	ecs::EntityObject object( find_entity() );

	return object.is_valid() ? object.get_location() : core::Vector::Zero;
}

void GameObjectRef::set_location( const core::Vector& location )
{
	if ( ActiveObject* o = get_active_object() )
	{
		o->set_location( location );
		return;
	}

	ecs::EntityObject object( find_entity() );

	if ( object.is_valid() )
	{
		object.set_location( location );
	}
}

void GameObjectRef::set_direction_degree( float_t degree )
{
	if ( ActiveObject* o = get_active_object() )
	{
		o->set_direction_degree( degree );
		return;
	}

	ecs::EntityObject object( find_entity() );

	if ( object.is_valid() )
	{
		object.set_direction_degree( degree );
	}
}

void GameObjectRef::set_velocity( const core::Vector& velocity )
{
	if ( ActiveObject* o = get_active_object() )
	{
		o->set_velocity( velocity );
		return;
	}

	ecs::EntityObject object( find_entity() );

	if ( object.is_valid() )
	{
		object.set_velocity( velocity );
	}
}

void GameObjectRef::set_start_location( float_t x, float_t y, float_t z )
{
	if ( ActiveObject* o = get_active_object() )
	{
		o->set_start_location( x, y, z );
		return;
	}

	ecs::EntityObject object( find_entity() );

	if ( object.is_valid() )
	{
		object.set_start_location( core::Vector( x, y, z ) );
	}
}

void GameObjectRef::set_start_rotation( float_t x, float_t y, float_t z )
{
	if ( ActiveObject* o = get_active_object() )
	{
		o->set_start_rotation( x, y, z );
		return;
	}

	ecs::EntityObject object( find_entity() );

	if ( object.is_valid() )
	{
		object.set_start_rotation( x, y, z );
	}
}

void GameObjectRef::set_start_direction_degree( float_t degree )
{
	if ( ActiveObject* o = get_active_object() )
	{
		o->set_start_direction_degree( degree );
		return;
	}

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
	if ( ActiveObject* o = get_active_object() )
	{
		o->set_mass( mass );
	}
	else if ( btRigidBody* body = find_rigid_body( find_entity() ) )
	{
		ecs::object_body::set_mass( * body, mass );
	}
}

void GameObjectRef::set_gravity( const core::Vector& gravity )
{
	if ( ActiveObject* o = get_active_object() )
	{
		o->set_gravity( gravity );
	}
	else if ( btRigidBody* body = find_rigid_body( find_entity() ) )
	{
		ecs::object_body::set_gravity( * body, gravity );
	}
}

void GameObjectRef::set_friction( float_t friction )
{
	if ( ActiveObject* o = get_active_object() )
	{
		o->set_friction( friction );
	}
	else if ( btRigidBody* body = find_rigid_body( find_entity() ) )
	{
		ecs::object_body::set_friction( * body, friction );
	}
}

void GameObjectRef::set_angular_factor( const core::Vector& factor )
{
	if ( ActiveObject* o = get_active_object() )
	{
		o->set_angular_factor( factor );
	}
	else if ( btRigidBody* body = find_rigid_body( find_entity() ) )
	{
		ecs::object_body::set_angular_factor( * body, factor );
	}
}

void GameObjectRef::set_kinematic( bool is_kinematic )
{
	if ( ActiveObject* o = get_active_object() )
	{
		o->set_kinematic( is_kinematic );
	}
	else if ( btRigidBody* body = find_rigid_body( find_entity() ) )
	{
		ecs::object_body::set_kinematic( * body, is_kinematic );
	}
}

void GameObjectRef::set_no_contact_response( bool is_no_contact_response )
{
	if ( ActiveObject* o = get_active_object() )
	{
		o->set_no_contact_response( is_no_contact_response );
	}
	else if ( btRigidBody* body = find_rigid_body( find_entity() ) )
	{
		ecs::object_body::set_no_contact_response( * body, is_no_contact_response );
	}
}

void GameObjectRef::set_flicker_scale( float_t scale )
{
	if ( ActiveObject* o = get_active_object() )
	{
		o->set_flicker_scale( scale );
		return;
	}

	// ゆらぐのはプレイヤー・少女・敵・風船（ActiveObject）だけ
	log_unsupported( "ゆらぎの大きさの設定" );
}

void GameObjectRef::play_animation( const char_t* name, bool force, bool loop )
{
	if ( ActiveObject* o = get_active_object() )
	{
		o->play_animation( name, force, loop );
		return;
	}

	core::ecs::Entity* entity = find_entity();

	if ( auto* animation = entity ? entity->get_component< ecs::AnimationComponent >() : nullptr )
	{
		animation->player->play( name, force, loop );
	}
}

void GameObjectRef::set_animation_speed( float_t speed )
{
	if ( ActiveObject* o = get_active_object() )
	{
		if ( o->get_animation_player() )
		{
			o->get_animation_player()->set_speed( speed );
		}

		return;
	}

	core::ecs::Entity* entity = find_entity();

	if ( auto* animation = entity ? entity->get_component< ecs::AnimationComponent >() : nullptr )
	{
		animation->player->set_speed( speed );
	}
}

void GameObjectRef::action( const string_t& action )
{
	if ( ActiveObject* o = get_active_object() )
	{
		o->action( action );
		return;
	}

	core::ecs::Entity* entity = find_entity();

	if ( ! entity )
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
	if ( ActiveObject* o = get_active_object() )
	{
		o->kill();
		return;
	}

	if ( core::ecs::Entity* entity = find_entity() )
	{
		ecs::EntityObject( entity ).kill();
	}
}

void GameObjectRef::restart()
{
	if ( ActiveObject* o = get_active_object() )
	{
		o->restart();
		return;
	}

	ecs::EntityObject object( find_entity() );

	if ( object.is_valid() )
	{
		object.restart();
	}
}

GameObjectRef::Model* GameObjectRef::get_model() const
{
	if ( ActiveObject* o = get_active_object() )
	{
		return o->get_model();
	}

	core::ecs::Entity* entity = find_entity();
	const auto* render = entity ? entity->get_component< ecs::ObjectRenderComponent >() : nullptr;

	return render && graphics_manager_ ? graphics_manager_->resolve_model( render->model ) : nullptr;
}

void GameObjectRef::set_model( Model* model )
{
	if ( ActiveObject* o = get_active_object() )
	{
		o->set_model( model );
		return;
	}

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
	if ( ActiveObject* o = get_active_object() )
	{
		return o->make_model_unique();
	}

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
	if ( ActiveObject* o = get_active_object() )
	{
		o->add_event_handler( name, handler );
		return;
	}

	log_unsupported( ( string_t( "イベント \"" ) + name + "\" の受け取り" ).c_str() );
}

void GameObjectRef::set_target_location( const core::Vector& location, float_t speed )
{
	auto* system = entity_manager_ ? entity_manager_->get_system< ecs::MoveToTargetSystem >() : nullptr;

	if ( ! system || ! is_alive() || ! system->set_target_location( id_, location, speed ) )
	{
		debug_log( "[Object] 目標地点を設定できません（物体の Entity が無い。ActiveObjectManager に加えていない・片付けた後）" );
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
