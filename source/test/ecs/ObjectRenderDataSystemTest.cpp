#include "pch.h"

#include <blue_sky/ecs/system/ObjectRenderDataSystem.h>
#include <blue_sky/ecs/component/ObjectRenderComponent.h>
#include <blue_sky/ecs/component/AnimationComponent.h>

#include <core/ecs/EntityManager.h>
#include <core/ecs/component/TransformComponent.h>
#include <core/math/Quaternion.h>

#include <common/math.h>

using blue_sky::ecs::ObjectRenderDataSystem;
using blue_sky::ecs::ObjectRenderComponent;
using blue_sky::ecs::AnimationComponent;
using core::ecs::EntityManager;
using core::ecs::TransformComponent;

namespace
{

/// 描画の部品（ObjectRenderComponent と TransformComponent）を持つ Entity を作る
core::ecs::Entity* create_render_entity( EntityManager& manager )
{
	auto* entity = manager.create_entity();
	entity->add_component< TransformComponent >();
	entity->add_component< ObjectRenderComponent >();
	return entity;
}

/// 2 つの行列がほぼ同じか（各行 = 基底ベクトルを写した先を比べる）
void expect_matrix_near( const core::Matrix& a, const core::Matrix& b )
{
	const core::Vector basis[] = { { 1.f, 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f, 0.f }, { 0.f, 0.f, 1.f, 0.f }, { 0.f, 0.f, 0.f, 1.f } };

	for ( int n = 0; n < 4; n++ )
	{
		const core::Vector va = basis[ n ] * a;
		const core::Vector vb = basis[ n ] * b;

		EXPECT_NEAR( va.x(), vb.x(), 1e-5f ) << "行 " << n;
		EXPECT_NEAR( va.y(), vb.y(), 1e-5f ) << "行 " << n;
		EXPECT_NEAR( va.z(), vb.z(), 1e-5f ) << "行 " << n;
		EXPECT_NEAR( va.w(), vb.w(), 1e-5f ) << "行 " << n;
	}
}

} // namespace

TEST( ObjectRenderDataSystemTest, 描画の部品と位置の両方を持つ_Entity_だけが描く対象になる )
{
	EntityManager manager;
	manager.add_system< ObjectRenderDataSystem >( 0, nullptr );
	auto* system = manager.get_system< ObjectRenderDataSystem >();

	create_render_entity( manager );

	auto* only_render = manager.create_entity();
	only_render->add_component< ObjectRenderComponent >();

	auto* only_transform = manager.create_entity();
	only_transform->add_component< TransformComponent >();

	EXPECT_EQ( 1u, system->get_render_targets().size() );
}

TEST( ObjectRenderDataSystemTest, 描く対象は作った順に並ぶ )
{
	EntityManager manager;
	manager.add_system< ObjectRenderDataSystem >( 0, nullptr );
	auto* system = manager.get_system< ObjectRenderDataSystem >();

	std::vector< core::ecs::EntityId > ids;

	for ( int n = 0; n < 50; n++ )
	{
		ids.push_back( create_render_entity( manager )->get_id() );
	}

	const auto& targets = system->get_render_targets();
	ASSERT_EQ( ids.size(), targets.size() );

	for ( size_t n = 0; n < ids.size(); n++ )
	{
		EXPECT_EQ( ids[ n ], targets[ n ].id );
	}
}

TEST( ObjectRenderDataSystemTest, Entity_を消すと描く対象から外れる )
{
	EntityManager manager;
	manager.add_system< ObjectRenderDataSystem >( 0, nullptr );
	auto* system = manager.get_system< ObjectRenderDataSystem >();

	auto* a = create_render_entity( manager );
	auto* b = create_render_entity( manager );
	const auto b_id = b->get_id();

	ASSERT_EQ( 2u, system->get_render_targets().size() );	// 一覧を一度作ってから消す（作り直されることを確かめる）

	manager.destroy_entity( a->get_id() );

	ASSERT_EQ( 1u, system->get_render_targets().size() );
	EXPECT_EQ( b_id, system->get_render_targets()[ 0 ].id );
}

TEST( ObjectRenderDataSystemTest, 部品を外すと描く対象から外れる )
{
	EntityManager manager;
	manager.add_system< ObjectRenderDataSystem >( 0, nullptr );
	auto* system = manager.get_system< ObjectRenderDataSystem >();

	auto* entity = create_render_entity( manager );
	ASSERT_EQ( 1u, system->get_render_targets().size() );

	entity->remove_component< ObjectRenderComponent >();

	EXPECT_EQ( 0u, system->get_render_targets().size() );
}

TEST( ObjectRenderDataSystemTest, System_を後から足しても既にある_Entity_が描く対象になる )
{
	// シーンの切り替えで EntityManager が System を片付けた後、ActiveObjectManager が足し直す場合
	EntityManager manager;
	create_render_entity( manager );

	manager.add_system< ObjectRenderDataSystem >( 0, nullptr );

	EXPECT_EQ( 1u, manager.get_system< ObjectRenderDataSystem >()->get_render_targets().size() );
}

TEST( ObjectRenderDataSystemTest, アニメーションの部品は後から足しても引ける )
{
	EntityManager manager;
	manager.add_system< ObjectRenderDataSystem >( 0, nullptr );
	auto* system = manager.get_system< ObjectRenderDataSystem >();

	auto* entity = create_render_entity( manager );
	EXPECT_EQ( nullptr, system->get_animation( entity->get_id() ) );

	auto* animation = entity->add_component< AnimationComponent >();
	EXPECT_EQ( animation, system->get_animation( entity->get_id() ) );

	manager.destroy_entity( entity->get_id() );
	EXPECT_EQ( nullptr, system->get_animation( entity->get_id() ) );
}

TEST( ObjectRenderDataSystemTest, GraphicsManager_が無ければ定数を書かず前フレームの_world_も進めない )
{
	EntityManager manager;
	manager.add_system< ObjectRenderDataSystem >( 0, nullptr );

	auto* entity = create_render_entity( manager );
	manager.get_system< ObjectRenderDataSystem >()->update_render_data();
	manager.get_system< ObjectRenderDataSystem >()->update_animations();	// アニメーションの部品が無くても落ちない

	EXPECT_FALSE( entity->get_component< ObjectRenderComponent >()->prev_world_valid );
}

TEST( ObjectRenderDataSystemTest, world_は回転してから移動する )
{
	core::Transform t;
	t.set_position( core::Vector( 1.f, 2.f, 3.f ) );
	t.set_rotation( core::Quaternion::from_yaw_pitch_roll( math::degree_to_radian( 90.f ), 0.f, 0.f ) );

	const core::Matrix world = ObjectRenderDataSystem::make_world_matrix( t );

	// 以前の ActiveObject::update_render_data と同じ式
	core::Matrix expected;
	expected.set_rotation_quaternion( t.get_rotation() );
	expected *= core::Matrix().set_translation( 1.f, 2.f, 3.f );

	expect_matrix_near( expected, world );

	// 原点は位置へ、前（+Z）は Y 軸まわりに 90 度回って +X へ
	const core::Vector origin = core::Vector( 0.f, 0.f, 0.f, 1.f ) * world;
	EXPECT_NEAR( 1.f, origin.x(), 1e-5f );
	EXPECT_NEAR( 2.f, origin.y(), 1e-5f );
	EXPECT_NEAR( 3.f, origin.z(), 1e-5f );

	const core::Vector front = core::Vector( 0.f, 0.f, 1.f, 0.f ) * world;
	EXPECT_NEAR( 1.f, front.x(), 1e-5f );
	EXPECT_NEAR( 0.f, front.z(), 1e-5f );
}

TEST( ObjectRenderDataSystemTest, 前フレームの_world_は初回は今の_world_で次からは前の値 )
{
	ObjectRenderComponent render;

	const core::Matrix first = core::Matrix().set_translation( 1.f, 0.f, 0.f );
	const core::Matrix second = core::Matrix().set_translation( 2.f, 0.f, 0.f );

	// 初回は今の world を前フレームの world にする（速度 0）
	expect_matrix_near( first, ObjectRenderDataSystem::advance_prev_world( render, first ) );
	EXPECT_TRUE( render.prev_world_valid );

	// 次は前のフレームの world
	expect_matrix_near( first, ObjectRenderDataSystem::advance_prev_world( render, second ) );
	expect_matrix_near( second, render.prev_world );
}
