#include "pch.h"

#include <core/ecs/EntityManager.h>
#include <core/ecs/component/TransformComponent.h>

#include <blue_sky/ecs/component/PerceivableComponent.h>
#include <blue_sky/ecs/component/PerceptionComponent.h>
#include <blue_sky/ecs/system/TargetRegistrySystem.h>
#include <blue_sky/ecs/system/PerceptionSystem.h>

// 知覚 System が、視界錐の内側で最も近い対象を見るかを確かめる（物理の世界は nullptr = 遮蔽なし）

namespace
{

using core::ecs::EntityManager;
using core::ecs::TransformComponent;
using blue_sky::ecs::PerceivableComponent;
using blue_sky::ecs::PerceptionComponent;
using blue_sky::ecs::TargetRegistrySystem;
using blue_sky::ecs::PerceptionSystem;

/// 知覚する敵（原点・既定の向き）を作る
core::ecs::Entity* create_enemy( EntityManager& manager )
{
	auto* e = manager.create_entity();
	e->add_component< TransformComponent >()->transform.set_identity();
	e->add_component< PerceptionComponent >();
	return e;
}

/// 知覚される対象を、敵の目の高さで、敵の前方 forward_distance の所に作る（負なら後ろ）
core::ecs::Entity* create_target( EntityManager& manager, const core::ecs::Entity* enemy, float forward_distance )
{
	const auto& enemy_transform = enemy->get_component< TransformComponent >()->transform;
	const float eye_height = enemy->get_component< PerceptionComponent >()->eye_height;

	auto* e = manager.create_entity();
	auto* t = e->add_component< TransformComponent >();
	t->transform.set_identity();
	t->transform.set_position( enemy_transform.get_position() + enemy_transform.forward() * forward_distance + core::math::Vector( 0.f, eye_height, 0.f ) );
	e->add_component< PerceivableComponent >();
	return e;
}

void setup_systems( EntityManager& manager )
{
	manager.add_system< TargetRegistrySystem >( 20 );
	manager.add_system< PerceptionSystem >( 30, nullptr );
}

} // namespace

TEST( PerceptionSystemTest, 視界錐の内側で最も近い対象を見る )
{
	EntityManager manager;
	setup_systems( manager );

	auto* enemy = create_enemy( manager );
	create_target( manager, enemy, 10.f );
	auto* nearest = create_target( manager, enemy, 5.f );
	create_target( manager, enemy, 15.f );
	create_target( manager, enemy, -3.f );	// 後ろ（より近いが見えない）

	manager.update();

	const auto* perception = enemy->get_component< PerceptionComponent >();
	ASSERT_TRUE( perception->target_visible );
	ASSERT_TRUE( perception->has_last_seen );

	const auto expected = nearest->get_component< TransformComponent >()->transform.get_position();
	EXPECT_NEAR( expected.x(), perception->last_seen_position.x(), 1e-4f );
	EXPECT_NEAR( expected.y(), perception->last_seen_position.y(), 1e-4f );
	EXPECT_NEAR( expected.z(), perception->last_seen_position.z(), 1e-4f );
}

TEST( PerceptionSystemTest, 対象が見えなくなれば見えないに戻り_最後に見た位置は残る )
{
	EntityManager manager;
	setup_systems( manager );

	auto* enemy = create_enemy( manager );
	auto* target = create_target( manager, enemy, 5.f );

	manager.update();
	ASSERT_TRUE( enemy->get_component< PerceptionComponent >()->target_visible );

	manager.destroy_entity( target->get_id() );
	create_target( manager, enemy, -5.f );	// 後ろだけ

	manager.update();

	const auto* perception = enemy->get_component< PerceptionComponent >();
	EXPECT_FALSE( perception->target_visible );
	EXPECT_TRUE( perception->has_last_seen );
}

TEST( PerceptionSystemTest, 対象の一覧が無ければ見えない )
{
	EntityManager manager;
	manager.add_system< PerceptionSystem >( 30, nullptr );

	auto* enemy = create_enemy( manager );
	enemy->get_component< PerceptionComponent >()->target_visible = true;

	manager.update();

	EXPECT_FALSE( enemy->get_component< PerceptionComponent >()->target_visible );
}

TEST( PerceptionSystemTest, 敵が複数でもそれぞれ判定する )
{
	EntityManager manager;
	setup_systems( manager );

	auto* a = create_enemy( manager );
	auto* b = create_enemy( manager );
	create_target( manager, a, 5.f );	// 2 体とも原点・同じ向きなので両方に見える

	manager.update();

	EXPECT_TRUE( a->get_component< PerceptionComponent >()->target_visible );
	EXPECT_TRUE( b->get_component< PerceptionComponent >()->target_visible );
}
