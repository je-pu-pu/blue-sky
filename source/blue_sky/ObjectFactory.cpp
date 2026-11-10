#include "ObjectFactory.h"

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
#include <blue_sky/ecs/component/ContactCacheComponent.h>
#include <blue_sky/ecs/component/SwitchComponent.h>
#include <blue_sky/ecs/component/TargetMoverComponent.h>
#include <blue_sky/ecs/system/CollisionEventSystem.h>
#include <blue_sky/ecs/system/MoveToTargetSystem.h>
#include <blue_sky/ecs/system/ObjectRenderDataSystem.h>
#include <blue_sky/ecs/system/PhysicsSyncSystem.h>
#include <blue_sky/ecs/system/TranslationSystem.h>
#include <blue_sky/ecs/system/SpinSystem.h>
#include <blue_sky/ecs/system/ContactCacheSystem.h>
#include <blue_sky/ecs/system/SwitchSystem.h>
#include <blue_sky/ecs/system/BalloonHoldSystem.h>
#include <blue_sky/ecs/component/BalloonHoldComponent.h>
#include <blue_sky/ecs/system/GirlSystem.h>
#include <blue_sky/ecs/component/GirlComponent.h>
#include <blue_sky/ecs/component/FlickerComponent.h>
#include <blue_sky/ecs/system/RobotBehaviorSystem.h>
#include <blue_sky/ecs/component/RobotBehaviorComponent.h>
#include <blue_sky/ecs/component/PatrolComponent.h>
#include <blue_sky/ecs/component/PlayerBehaviorComponent.h>
#include <blue_sky/ecs/component/PlayerInputComponent.h>
#include <blue_sky/ecs/system/PlayerBehaviorSystem.h>
#include <blue_sky/ecs/system/PlayerInputSystem.h>
#include <blue_sky/graphics/Line.h>

#include <core/ecs/EntityManager.h>
#include <core/ecs/component/TransformComponent.h>
#include <core/physics/EntityTag.h>

#include <cstdio>
#include <map>
#include <set>
#include <typeinfo>

namespace blue_sky
{

ObjectFactory::ObjectFactory( core::ecs::EntityManager* entity_manager, ActiveObjectPhysics* physics, graphics::GraphicsManager* graphics_manager, core::sound::SoundManager* sound_manager )
	: entity_manager_( entity_manager )
	, physics_( physics )
	, graphics_manager_( graphics_manager )
	, sound_manager_( sound_manager )
{

}

void ObjectFactory::setup_systems()
{
	// 物体を扱う System（描く・目標へ動かす・当たり・剛体の位置の同期・種類ごとの振る舞い）
	entity_manager_->add_system< ecs::ObjectRenderDataSystem >( 0, graphics_manager_ );
	entity_manager_->add_system< ecs::MoveToTargetSystem >( 0 );
	entity_manager_->add_system< ecs::CollisionEventSystem >( 0 );
	entity_manager_->add_system< ecs::PhysicsSyncSystem >( 0 );
	entity_manager_->add_system< ecs::TranslationSystem >( 0 );
	entity_manager_->add_system< ecs::SpinSystem >( 0 );
	entity_manager_->add_system< ecs::ContactCacheSystem >( 0 );
	entity_manager_->add_system< ecs::SwitchSystem >( 0, sound_manager_ );
	entity_manager_->add_system< ecs::BalloonHoldSystem >( 0, sound_manager_ );
	entity_manager_->add_system< ecs::GirlSystem >( 0 );
	entity_manager_->add_system< ecs::RobotBehaviorSystem >( 0, physics_, graphics_manager_, sound_manager_ );
	entity_manager_->add_system< ecs::PlayerBehaviorSystem >( 0, physics_, graphics_manager_, sound_manager_ );
	entity_manager_->add_system< ecs::PlayerInputSystem >( 0 );
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

	if ( kind == "goal" )
	{
		// 以前の Goal : 4 x 2 x 4・硬い・着地しても死なない足場・重力 0・やり直すと質量 100（作ったときは 1）。
		// 当たりでは何も起こさない（ゴールしたかはシーンがプレイヤーとの接触で調べる）。目標地点に着いたら止まる
		ObjectSpec spec;
		spec.kind = kind;
		spec.size = Vector( 4.f, 2.f, 4.f );
		spec.mass = 1.f;
		spec.is_safe_footing = true;
		spec.has_default_gravity = true;
		spec.gravity = Vector::Zero;
		spec.restart_mass = 100.f;

		return create_entity_object( spec );
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

	if ( kind == "stone" )
	{
		// 以前の Stone : 0.2 x 0.2 x 0.2・硬い（ぶつかる）が壁にはならない・重力 ( 0, -8, 0 )・質量 1・1 フレームに 1 度回る。
		// プレイヤーが触れると拾われて消え、投げられると生き返る。敵・風船・スイッチに当たると相手が反応する
		ObjectSpec spec;
		spec.kind = kind;
		spec.size = Vector( 0.2f, 0.2f, 0.2f );
		spec.mass = 1.f;
		spec.is_hard = true;
		spec.is_block = false;
		spec.has_default_gravity = true;
		spec.gravity = Vector( 0.f, -8.f, 0.f );

		const EntityId id = create_entity_object( spec );
		core::ecs::Entity* entity = entity_manager_->find( id );

		entity->add_component< ecs::ThrowableComponent >();
		entity->add_component< ecs::SpinComponent >()->degree_per_frame = 1.f;

		return id;
	}

	if ( kind == "switch" )
	{
		// 以前の Switch : 0.6 x 0.9 x 0.6・硬い・壁になる・地球の重力・作ったときの質量 1。やり直すと 切 に戻り、
		// "Off" のアニメーション・回らない（回転のしやすさ 0）・摩擦 1.5・質量 100。プレイヤーと速く飛ぶ石で入・切が変わる
		ObjectSpec spec;
		spec.kind = kind;
		spec.size = Vector( 0.6f, 0.9f, 0.6f );
		spec.mass = 1.f;
		spec.restart_mass = 100.f;
		spec.restart_friction = 1.5f;
		spec.restart_angular_factor = 0.f;

		const EntityId id = create_entity_object( spec );
		core::ecs::Entity* entity = entity_manager_->find( id );

		entity->add_component< ecs::SwitchComponent >();
		entity->add_component< ecs::ContactCacheComponent >();

		return id;
	}

	if ( kind == "balloon" )
	{
		// 以前の Balloon : 1.5 x 1.5 x 1.5・硬くない（当たりの応答なし・重力 0・壁にならない）・着地しても死なない足場・足元と剛体の中心が同じ・質量 1。
		// 始めの位置でふわふわと上下しながら回り、プレイヤーが触れるとつかまれて手元に付いて行く（BalloonHoldSystem）。石が当たると割れる
		ObjectSpec spec;
		spec.kind = kind;
		spec.size = Vector( 1.5f, 1.5f, 1.5f );
		spec.mass = 1.f;
		spec.is_hard = false;
		spec.is_block = false;
		spec.is_safe_footing = true;
		spec.height_offset = 0.f;

		const EntityId id = create_entity_object( spec );
		entity_manager_->find( id )->add_component< ecs::BalloonHoldComponent >();

		// 線に影を落とさせる（以前の Balloon::set_model。モデルは風船どうしで共有している）
		if ( Model* model = get_ref( id ).get_model(); model && model->get_line() )
		{
			model->get_line()->set_cast_shadow( true );
		}

		return id;
	}

	if ( kind == "girl" )
	{
		// 以前の Girl : 0.5 x 1.5 x 0.5・硬い・重力 0・質量 1。やり直すと回らない（回転のしやすさ 0）・摩擦 0 で立つ。
		// アクション "float" で浮き、浮く中心でふわふわと上下する（GirlSystem）。当たりでは何も起こさない
		ObjectSpec spec;
		spec.kind = kind;
		spec.size = Vector( 0.5f, 1.5f, 0.5f );
		spec.mass = 1.f;
		spec.has_default_gravity = true;
		spec.gravity = Vector::Zero;
		spec.restart_angular_factor = 0.f;
		spec.restart_friction = 0.f;

		const EntityId id = create_entity_object( spec );
		core::ecs::Entity* entity = entity_manager_->find( id );

		entity->add_component< ecs::GirlComponent >();
		entity->add_component< ecs::FlickerComponent >();

		return id;
	}

	if ( kind == "robot" )
	{
		// 以前の Robot : 1 x 2 x 0.5・硬い・地球の重力・作ったときの質量 1。やり直すと質量 50・回らない・摩擦 0 で立つ。
		// 行動（立つ・回る・注目・追う・止まる・浮く）は RobotBehaviorComponent にまとめて持ち、RobotBehaviorSystem が動かす。追う相手は set_robot_target で渡す
		ObjectSpec spec;
		spec.kind = kind;
		spec.size = Vector( 1.f, 2.f, 0.5f );
		spec.mass = 1.f;
		spec.restart_mass = 50.f;
		spec.restart_angular_factor = 0.f;
		spec.restart_friction = 0.f;

		const EntityId id = create_entity_object( spec );
		core::ecs::Entity* entity = entity_manager_->find( id );

		entity->add_component< ecs::RobotBehaviorComponent >();
		entity->add_component< ecs::PatrolComponent >()->move_speed = ecs::RobotBehaviorComponent::PatrolSpeed;
		entity->add_component< ecs::FlickerComponent >();

		if ( auto* system = entity_manager_->get_system< ecs::RobotBehaviorSystem >() )
		{
			system->load_textures( id );
		}

		return id;
	}

	if ( kind == "ladder" )
	{
		// 以前の Ladder : 0.4 x 5 x 0.2・硬くない（当たりの応答なし・重力 0・壁にならない）・作ったときの質量 1、やり直すと 0（動かない）。
		// プレイヤーが触れると梯子につかまり、梯子は（最近触れていなければ）触れた音を鳴らす
		ObjectSpec spec;
		spec.kind = kind;
		spec.size = Vector( 0.4f, 5.f, 0.2f );
		spec.mass = 1.f;
		spec.is_hard = false;
		spec.is_block = false;
		spec.restart_mass = 0.f;

		const EntityId id = create_entity_object( spec );
		core::ecs::Entity* entity = entity_manager_->find( id );

		entity->add_component< ecs::LadderComponent >();
		entity->add_component< ecs::ContactCacheComponent >();

		return id;
	}

	// 失敗を黙って無視しない（CLAUDE.md の「失敗の扱い」）。プレイヤーは create_player で作る（名前からは作らない。以前もシーンが作っていた）
	debug_log( "[Object] 知らないクラス名のオブジェクトは作れません: " + kind );

	return std::nullopt;
}

ObjectFactory::EntityId ObjectFactory::create_static_object( std::stringstream& ss )
{
	string_t object_name;
	float_t x = 0, y = 0, z = 0, rx = 0, ry = 0, rz = 0;

	ss >> object_name >> x >> y >> z >> rx >> ry >> rz;

	return create_static_object( object_name, Vector( x, y, z ), Vector( rx, ry, rz ) );
}

ObjectFactory::EntityId ObjectFactory::create_static_object( const string_t& object_name, const Vector& location, const Vector& rotation )
{
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
	spec.location = location;
	spec.rotation = rotation;
	spec.direction_degree = rotation.x();		// 以前と同じ（4 つ目の数が向きになる）

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
	state->default_gravity = spec.has_default_gravity ? spec.gravity : ( spec.is_hard ? ecs::object_body::GravityDefault : Vector::Zero );
	state->restart_mass = spec.restart_mass;
	state->restart_friction = spec.restart_friction;
	state->restart_angular_factor = spec.restart_angular_factor;

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
		traits->is_ghost = spec.is_ghost;
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

ObjectFactory::EntityId ObjectFactory::create_area_switch( float_t w, float_t h, float_t d )
{
	ObjectSpec spec;
	spec.kind = "area-switch";
	spec.size = Vector( w, h, d );
	spec.mass = 1.f;
	spec.is_hard = false;
	spec.is_block = false;
	spec.is_ghost = true;
	spec.height_offset = 0.f;
	spec.load_model = false;		// 以前もモデルを設定していなかった（描かない）

	const EntityId id = create_entity_object( spec );
	core::ecs::Entity* entity = entity_manager_->find( id );

	auto* s = entity->add_component< ecs::SwitchComponent >();
	s->is_switched_by_stone = false;
	s->has_feedback = false;

	entity->add_component< ecs::ContactCacheComponent >();

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

std::vector< ObjectFactory::EntityId > ObjectFactory::find_all_of_kind( const string_t& kind ) const
{
	std::vector< EntityId > result;

	for ( const EntityId id : entity_object_list_ )
	{
		const core::ecs::Entity* entity = entity_manager_->find( id );
		const auto* state = entity ? entity->get_component< ecs::SimpleObjectComponent >() : nullptr;

		if ( state && state->kind == kind )
		{
			result.push_back( id );
		}
	}

	return result;
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

void ObjectFactory::set_robot_target( EntityId robot, EntityId target_id )
{
	if ( auto* system = entity_manager_->get_system< ecs::RobotBehaviorSystem >() )
	{
		system->set_target( robot, target_id );
	}
}

ObjectFactory::EntityId ObjectFactory::create_player()
{
	// 以前の Player : 0.5 x 1.75 x 0.5・硬い・壁になる・地球の重力・作ったときの質量 1（やり直すと 50・回らない・摩擦 0。PlayerBehaviorSystem::restart）
	ObjectSpec spec;
	spec.kind = "player";
	spec.size = Vector( ecs::PlayerBehaviorComponent::CollisionWidth, ecs::PlayerBehaviorComponent::CollisionHeight, ecs::PlayerBehaviorComponent::CollisionDepth );
	spec.mass = 1.f;
	spec.shape = Shape::NONE;		// 剛体はステージを読むときに作る（add_player_rigid_body）

	const EntityId id = create_entity_object( spec );
	core::ecs::Entity* entity = entity_manager_->find( id );

	entity->add_component< ecs::PlayerBehaviorComponent >();
	entity->add_component< ecs::PlayerInputComponent >();
	entity->add_component< ecs::FlickerComponent >();

	return id;
}

void ObjectFactory::add_player_rigid_body( EntityId player )
{
	core::ecs::Entity* entity = entity_manager_->find( player );

	if ( ! physics_ || ! entity || ! entity->get_component< ecs::PlayerBehaviorComponent >() )
	{
		return;
	}

	const float_t w = ecs::PlayerBehaviorComponent::CollisionWidth;
	const float_t h = ecs::PlayerBehaviorComponent::CollisionHeight;
	const float_t d = ecs::PlayerBehaviorComponent::CollisionDepth;

	const float_t radius = std::max( w, d ) / 2.f;
	const float_t height = h - ( radius * 2.f );

	Transform offset;
	offset.set_identity();
	offset.set_position( Vector( 0, h / 2, 0 ) );

	auto* transform = entity->get_component< core::ecs::TransformComponent >();
	core::physics::RigidBody owner = physics_->add_capsule_rigid_body( transform->transform, offset, radius, height, 1.f );
	btRigidBody* body = owner.get();

	auto* body_component = entity->get_component< ecs::PhysicsBodyComponent >();

	if ( ! body_component )
	{
		body_component = entity->add_component< ecs::PhysicsBodyComponent >();
	}

	body_component->owner = std::move( owner );		// 前の剛体があれば、ここで世界から外れて消える
	body_component->body = body;
	body_component->height_offset = h * 0.5f;

	core::physics::set_entity_tag( body, player );
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

std::vector< string_t > ObjectFactory::describe_objects() const
{
	// 番号 → 種類（作った種類の名前。段階 6 まではプレイヤーが ActiveObject で、型の名前 "class blue_sky::Player" だった）
	std::map< EntityId, string_t > kinds;

	for ( const EntityId id : entity_object_list_ )
	{
		const core::ecs::Entity* entity = entity_manager_->find( id );
		const auto* state = entity ? entity->get_component< ecs::SimpleObjectComponent >() : nullptr;

		if ( state )
		{
			kinds[ id ] = state->kind;
		}
	}

	// 番号 → 名前（名前の順）
	std::map< EntityId, std::set< string_t > > names;

	for ( const auto& [ name, id ] : named_object_map_ )
	{
		names[ id ].insert( name );
	}

	std::vector< string_t > lines;

	for ( const auto& [ id, kind ] : kinds )
	{
		const GameObjectRef o = get_ref( id );
		const Vector l = o.get_location();

		char buffer[ 256 ];
		std::snprintf( buffer, sizeof( buffer ), "#%u %s 位置 ( %.3f, %.3f, %.3f ) 向き %.3f", static_cast< unsigned >( id ), kind.c_str(), l.x(), l.y(), l.z(), o.get_direction_degree() );

		string_t line = buffer;

		if ( auto i = names.find( id ); i != names.end() )
		{
			line += " 名前";

			for ( const auto& name : i->second )
			{
				line += " " + name;
			}
		}

		const core::ecs::Entity* entity = entity_manager_->find( id );

		if ( const auto* s = entity ? entity->get_component< ecs::SwitchComponent >() : nullptr )
		{
			// イベントの名前の順に、受け取り手の数
			std::map< string_t, size_t > counts;

			for ( const auto& [ event_name, handlers ] : s->event_handler_map )
			{
				counts[ event_name ] = handlers.size();
			}

			line += " スイッチ";

			for ( const auto& [ event_name, count ] : counts )
			{
				line += " " + event_name + "=" + std::to_string( count );
			}
		}

		lines.push_back( line );
	}

	return lines;
}

void ObjectFactory::restart()
{
	// 目標地点・向きを全部捨てる（以前の ActiveObjectManager::restart）
	if ( auto* system = entity_manager_->get_system< ecs::MoveToTargetSystem >() )
	{
		system->clear_targets();
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
}

} // namespace blue_sky
