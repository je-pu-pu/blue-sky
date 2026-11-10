#include "ObjectFactory.h"

#include <blue_sky/ActiveObjectManager.h>
#include <blue_sky/ActiveObjectPhysics.h>
#include <blue_sky/ConstantBuffers.h>
#include <blue_sky/DebugLog.h>
#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/graphics/Model.h>

#include <blue_sky/ecs/EntityObject.h>
#include <blue_sky/ecs/ObjectBody.h>
#include <blue_sky/ecs/component/CollisionTraitsComponent.h>
#include <blue_sky/ecs/component/ObjectRenderComponent.h>
#include <blue_sky/ecs/component/PhysicsBodyComponent.h>
#include <blue_sky/ecs/component/SimpleObjectComponent.h>
#include <blue_sky/ecs/component/StaticObjectComponent.h>
#include <blue_sky/ecs/component/PickupComponent.h>
#include <blue_sky/ecs/component/TargetMoverComponent.h>
#include <blue_sky/ecs/system/CollisionEventSystem.h>
#include <blue_sky/ecs/system/MoveToTargetSystem.h>
#include <blue_sky/ecs/system/ObjectRenderDataSystem.h>
#include <blue_sky/ecs/system/PhysicsSyncSystem.h>
#include <blue_sky/ecs/system/TranslationSystem.h>
#include <blue_sky/ecs/system/SpinSystem.h>

#include <GameObject/ActiveObject.h>

#include <core/ecs/EntityManager.h>
#include <core/ecs/component/TransformComponent.h>
#include <core/physics/EntityTag.h>

namespace blue_sky
{

ObjectFactory::ObjectFactory( core::ecs::EntityManager* entity_manager, ActiveObjectManager* active_object_manager, ActiveObjectPhysics* physics, graphics::GraphicsManager* graphics_manager )
	: entity_manager_( entity_manager )
	, active_object_manager_( active_object_manager )
	, physics_( physics )
	, graphics_manager_( graphics_manager )
{

}

void ObjectFactory::setup_systems()
{
	// ActiveObjectManager::add_active_object と同じ System（ECS だけの物体も、描く・目標へ動かす・当たり・剛体の位置の同期は同じ System で扱う）
	entity_manager_->add_system< ecs::ObjectRenderDataSystem >( 0, graphics_manager_ );
	entity_manager_->add_system< ecs::MoveToTargetSystem >( 0 );
	entity_manager_->add_system< ecs::CollisionEventSystem >( 0 );
	entity_manager_->add_system< ecs::PhysicsSyncSystem >( 0 );
	entity_manager_->add_system< ecs::TranslationSystem >( 0 );
	entity_manager_->add_system< ecs::SpinSystem >( 0 );
}

std::optional< ObjectFactory::EntityId > ObjectFactory::create_object( const string_t& kind )
{
	if ( kind == "static" )
	{
		// 以前の StaticObject()（大きさ 0・質量 0 の箱）。モデルは種類の名前で読む（以前と同じ。スクリプトが後で差し替える）
		StaticObjectSpec spec;
		spec.kind = kind;

		return create_static_object( spec );
	}

	if ( kind == "medal" )
	{
		// 以前の Medal : 2 x 2 x 2・硬くない（当たりの応答なし・重力 0）・足元と剛体の中心が同じ・1 秒に 90 度回る。プレイヤーが拾う
		ObjectSpec spec;
		spec.kind = kind;
		spec.size = Vector( 2.f, 2.f, 2.f );
		spec.mass = 1.f;
		spec.is_hard = false;
		spec.is_block = false;
		spec.height_offset = 0.f;

		const EntityId id = create_entity_object( spec );
		core::ecs::Entity* entity = entity_manager_->find( id );

		entity->add_component< ecs::PickupComponent >()->kind = ecs::PickupKind::MEDAL;
		entity->add_component< ecs::SpinComponent >()->degree_per_second = 90.f;

		return id;
	}

	if ( kind == "rocket" )
	{
		// 以前の Rocket : 0.2 x 1 x 0.2・硬い（ぶつかる）が壁にはならない・地球の重力。
		// プレイヤーが触れると持ち物が増え、ロケットは消えて当たりの応答も消える（やり直すと戻る）
		ObjectSpec spec;
		spec.kind = kind;
		spec.size = Vector( 0.2f, 1.f, 0.2f );
		spec.mass = 1.f;
		spec.is_hard = true;
		spec.is_block = false;

		const EntityId id = create_entity_object( spec );
		auto* pickup = entity_manager_->find( id )->add_component< ecs::PickupComponent >();
		pickup->kind = ecs::PickupKind::ROCKET;
		pickup->disable_contact_on_pickup = true;

		return id;
	}

	if ( kind == "umbrella" )
	{
		// 以前の Umbrella : 2 x 2 x 2・硬くない（当たりの応答なし・重力 0）。プレイヤーが触れると消える（持ち物は増えない）
		ObjectSpec spec;
		spec.kind = kind;
		spec.size = Vector( 2.f, 2.f, 2.f );
		spec.mass = 1.f;
		spec.is_hard = false;
		spec.is_block = false;

		const EntityId id = create_entity_object( spec );
		entity_manager_->find( id )->add_component< ecs::PickupComponent >()->kind = ecs::PickupKind::UMBRELLA;

		return id;
	}

	ActiveObject* active_object = create_active_object( kind );

	if ( ! active_object )
	{
		return std::nullopt;
	}

	return active_object->get_entity_id();
}

ActiveObject* ObjectFactory::create_active_object( const string_t& kind )
{
	ActiveObject* active_object = active_object_manager_ ? active_object_manager_->create_object( kind ) : nullptr;

	// 失敗を黙って無視しない（CLAUDE.md の「失敗の扱い」）
	if ( ! active_object )
	{
		debug_log( "[Object] 知らないクラス名のオブジェクトは作れません: " + kind );
		return nullptr;
	}

	if ( physics_ )
	{
		active_object->set_rigid_body( physics_->add_active_object_as_box( active_object ) );
	}

	if ( graphics_manager_ )
	{
		active_object->set_model( graphics_manager_->load_model( kind.c_str() ) );
	}

	// モデルとシェーダーは同じ種類のオブジェクトで共有する（load_model が名前で使い回す）。
	// 1 つだけシェーダーを変えたいときは、そのオブジェクトの make_model_unique()（スクリプトでは make_model_unique( o )）で自分だけのものにする

	return active_object;
}

ObjectFactory::EntityId ObjectFactory::create_static_object( std::stringstream& ss )
{
	string_t object_name;
	float_t x = 0, y = 0, z = 0, rx = 0, ry = 0, rz = 0;

	ss >> object_name >> x >> y >> z >> rx >> ry >> rz;

	// 種類ごとの大きさと質量（以前の ActiveObjectManager::create_static_object と同じ。無い種類は大きさ 0・質量 0）
	static const std::unordered_map< string_t, Vector > size_map =
	{
		{ "soda-can-1",      Vector(  0.07f, 0.12f, 0.07f ) },
		{ "wall-1",          Vector(  4.f,   1.75f, 0.2f  ) },
		{ "wall-2",          Vector(  8.f,   2.5f,  0.2f  ) },
		{ "wall-3",          Vector(  4.f,   2.5f,  0.2f  ) },
		{ "outdoor-unit",    Vector(  0.7f,  0.6f,  0.24f ) },
		{ "building-20",     Vector( 10.f,  20.f,  10.f   ) },
		{ "building-200",    Vector( 80.f, 200.f,  60.f   ) },
		{ "board-1",         Vector(  4.f,   0.2f,  0.8f  ) },
		{ "box-5x5x5",       Vector(  5.f,   5.f,   5.f   ) },
		{ "box-2x2x2",       Vector(  2.f,   2.f,   2.f   ) },
		{ "robot-dead-body", Vector(  0.8f,  2.f,   0.5f  ) },
	};

	static const std::unordered_map< string_t, float_t > mass_map =
	{
		{ "soda-can-1",      50.f  },
		{ "board-1",         20.f  },
		{ "box-5x5x5",       1.f   },
		{ "box-2x2x2",       100.f },
		{ "robot-dead-body", 10.f  },
		{ "wall-3",          100.f },
	};

	StaticObjectSpec spec;
	spec.kind = object_name;
	spec.shape = object_name == "soda-can-1" ? Shape::CYLINDER : Shape::BOX;
	spec.location = Vector( x, y, z );
	spec.rotation = Vector( rx, ry, rz );
	spec.direction_degree = rx;		// 以前と同じ（4 つ目の数が向きになる）

	if ( auto i = size_map.find( object_name ); i != size_map.end() )
	{
		spec.size = i->second;
	}

	if ( auto i = mass_map.find( object_name ); i != mass_map.end() )
	{
		spec.mass = i->second;
	}

	const EntityId id = create_static_object( spec );

	if ( object_name == "box-2x2x2" )
	{
		get_ref( id ).set_friction( 10.f );
	}

	return id;
}

ObjectFactory::EntityId ObjectFactory::create_static_object( const StaticObjectSpec& spec )
{
	ObjectSpec s = spec;
	s.is_static_object = true;

	return create_entity_object( s );
}

ObjectFactory::EntityId ObjectFactory::create_entity_object( const ObjectSpec& spec )
{
	setup_systems();

	core::ecs::Entity* entity = entity_manager_->create_entity();
	const EntityId id = static_cast< EntityId >( entity->get_id() );

	// 位置の正（Transform の既定の回転は 0 なので、単位にしてから入れる）
	auto* transform = entity->add_component< core::ecs::TransformComponent >();
	transform->transform.set_identity();

	auto* state = entity->add_component< ecs::SimpleObjectComponent >();
	state->kind = spec.kind;
	// やり直したときの重力（以前の GameObject::get_default_gravity : 硬い物は地球の重力、硬くない物は 0。種類によって上書き）
	state->default_gravity = spec.has_default_gravity ? spec.gravity : ( spec.is_hard ? GameObject::GravityDefault : Vector::Zero );
	state->restart_mass = spec.restart_mass;

	{
		ecs::EntityObject object( entity );
		object.set_start_location( spec.location );
		object.set_start_rotation( spec.rotation.x(), spec.rotation.y(), spec.rotation.z() );
		object.set_start_direction_degree( spec.direction_degree );
	}

	// 剛体（以前の ActiveObjectPhysics::add_active_object_as_box・cylinder と同じ大きさ・ずれ。硬くない物は当たりの応答を消す）
	if ( physics_ && spec.shape != Shape::NONE )
	{
		const float_t w = spec.size.x(), h = spec.size.y(), d = spec.size.z();

		Transform offset;
		offset.set_identity();
		offset.set_position( Vector( 0, h / 2, 0 ) );

		const Vector half( w / 2.f, h / 2.f, d / 2.f );

		core::physics::RigidBody owner = spec.shape == Shape::CYLINDER
			? physics_->add_cylinder_rigid_body( transform->transform, offset, half, spec.mass )
			: physics_->add_box_rigid_body( transform->transform, offset, half, spec.mass );

		btRigidBody* body = owner.get();

		if ( ! spec.is_hard )
		{
			ecs::object_body::set_no_contact_response( * body, true );	// 以前の ActiveObjectPhysics::set_active_object_info_to_rigid_body
		}

		if ( spec.is_static_object )
		{
			ecs::object_body::set_mass( * body, spec.mass );			// 以前の create_static_object は作った後にもう一度設定していた（同じ値）
		}

		auto* body_component = entity->add_component< ecs::PhysicsBodyComponent >();
		body_component->owner = std::move( owner );
		body_component->body = body;
		body_component->height_offset = spec.height_offset >= 0.f ? spec.height_offset : h * 0.5f;	// 以前の GameObject::get_height_offset（既定は当たりの高さの半分）

		core::physics::set_entity_tag( body, id );
	}

	// 目標地点・向きへ動かす（ActiveObject を持たないので、MoveToTargetSystem が部品を直接動かす）
	entity->add_component< ecs::TargetMoverComponent >();

	// 当たりの性質（以前は種類ごとのクラスの仮想関数）と、動かない物の印
	{
		auto* traits = entity->add_component< ecs::CollisionTraitsComponent >();
		traits->is_hard = spec.is_hard;
		traits->is_block = spec.is_block;
		traits->is_safe_footing = spec.is_safe_footing;
	}

	if ( spec.is_static_object )
	{
		entity->add_component< ecs::StaticObjectComponent >();
	}

	auto* render = entity->add_component< ecs::ObjectRenderComponent >();

	if ( graphics_manager_ )
	{
		// 毎フレーム書き換える（world・前フレームの world）ので DYNAMIC にする
		render->object_constant_buffer = graphics_manager_->create_constant_buffer( sizeof( ObjectConstantBufferData ), ObjectConstantBuffer::SLOT, core::graphics::ConstantBufferUsage::DYNAMIC );
	}

	entity_object_list_.push_back( id );

	if ( graphics_manager_ && spec.load_model )
	{
		get_ref( id ).set_model( graphics_manager_->load_model( spec.kind.c_str() ) );
	}

	return id;
}

ObjectFactory::EntityId ObjectFactory::create_translation_object( const core::Vector& location, float_t width, float_t height, float_t depth, float_t speed )
{
	StaticObjectSpec spec;
	spec.kind = "box-5x5x5";
	spec.size = Vector( 5.f, 5.f, 5.f );
	spec.location = location;

	const EntityId id = create_static_object( spec );

	auto* translation = entity_manager_->find( id )->add_component< ecs::TranslationComponent >();
	translation->width = width;
	translation->height = height;
	translation->depth = depth;
	translation->speed = speed;

	return id;
}

ObjectFactory::EntityId ObjectFactory::create_model_object( Model* model )
{
	StaticObjectSpec spec;
	spec.kind = "model";
	spec.shape = Shape::NONE;
	spec.load_model = false;

	const EntityId id = create_static_object( spec );

	get_ref( id ).set_model( model );

	return id;
}

std::optional< ObjectFactory::EntityId > ObjectFactory::find_first_of_kind( const string_t& kind ) const
{
	for ( const EntityId id : entity_object_list_ )
	{
		const core::ecs::Entity* entity = entity_manager_->find( id );
		const auto* state = entity ? entity->get_component< ecs::SimpleObjectComponent >() : nullptr;

		if ( state && state->kind == kind )
		{
			return id;
		}
	}

	return std::nullopt;
}

void ObjectFactory::name( const string_t& name, EntityId id )
{
	named_object_map_[ name ] = id;
}

std::optional< ObjectFactory::EntityId > ObjectFactory::find( const string_t& name ) const
{
	auto i = named_object_map_.find( name );

	if ( i == named_object_map_.end() || ! entity_manager_->is_alive( i->second ) )
	{
		return std::nullopt;
	}

	return i->second;
}

GameObjectRef ObjectFactory::get_ref( EntityId id ) const
{
	return GameObjectRef( entity_manager_, graphics_manager_, id );
}

std::optional< GameObjectRef > ObjectFactory::find_ref( const string_t& name ) const
{
	if ( auto id = find( name ) )
	{
		return get_ref( * id );
	}

	return std::nullopt;
}

void ObjectFactory::restart()
{
	if ( active_object_manager_ )
	{
		active_object_manager_->restart();
	}

	for ( const EntityId id : entity_object_list_ )
	{
		ecs::EntityObject object( entity_manager_->find( id ) );

		if ( object.is_valid() )
		{
			object.restart();
		}
	}
}

void ObjectFactory::clear()
{
	for ( const EntityId id : entity_object_list_ )
	{
		if ( entity_manager_->is_alive( id ) )
		{
			entity_manager_->destroy_entity( id );
		}
	}

	entity_object_list_.clear();
	named_object_map_.clear();

	if ( active_object_manager_ )
	{
		active_object_manager_->clear();
	}
}

} // namespace blue_sky
