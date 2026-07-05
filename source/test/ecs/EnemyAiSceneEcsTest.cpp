#include "pch.h"

// EnemyAiTestScene の ECS 自動登録が実機で全滅する問題を、段階的に切り分ける。

#include <core/ecs/EntityManager.h>

#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/component/CameraComponent.h>
#include <core/ecs/component/RigidBodyComponent.h>
#include <core/ecs/component/ModelComponent.h>
#include <core/ecs/component/RenderComponent.h>

#include <blue_sky/ecs/component/FpsPlayerComponent.h>
#include <blue_sky/ecs/component/PerceivableComponent.h>
#include <blue_sky/ecs/component/PerceptionComponent.h>
#include <blue_sky/ecs/component/AlertComponent.h>
#include <blue_sky/ecs/component/PatrolComponent.h>

#include <core/ecs/system/PhysicsSystem.h>
#include <blue_sky/ecs/system/FpsPlayerControlSystem.h>
#include <blue_sky/ecs/system/TargetRegistrySystem.h>
#include <blue_sky/ecs/system/PerceptionSystem.h>
#include <blue_sky/ecs/system/AlertSystem.h>
#include <blue_sky/ecs/system/PatrolSystem.h>

// 段階1: PhysicsSystem 単独。実体 Transform+RigidBody を後付け。
TEST( EnemyAiSceneEcsTest, Stage1_PhysicsAloneRealClass )
{
	auto em = core::ecs::EntityManager::get_instance();
	em->clear();
	em->add_system< core::ecs::PhysicsSystem >( 5 );
	auto* e = em->create_entity();
	e->add_component< core::ecs::TransformComponent >();
	e->add_component< core::ecs::RigidBodyComponent >();
	EXPECT_EQ( 1u, em->get_system< core::ecs::PhysicsSystem >()->debug_entity_count() );
	em->clear();
}

// 段階2: 自作 System 単独（PatrolSystem, 4要求）。
TEST( EnemyAiSceneEcsTest, Stage2_PatrolAlone )
{
	auto em = core::ecs::EntityManager::get_instance();
	em->clear();
	em->add_system< blue_sky::ecs::PatrolSystem >( 50 );
	auto* e = em->create_entity();
	e->add_component< core::ecs::TransformComponent >();
	e->add_component< blue_sky::ecs::PerceptionComponent >();
	e->add_component< blue_sky::ecs::AlertComponent >();
	e->add_component< blue_sky::ecs::PatrolComponent >();
	EXPECT_EQ( 1u, em->get_system< blue_sky::ecs::PatrolSystem >()->debug_entity_count() );
	em->clear();
}

// 段階3: TargetRegistrySystem 単独（update() をオーバーライドしている System）。
TEST( EnemyAiSceneEcsTest, Stage3_TargetRegistryAlone )
{
	auto em = core::ecs::EntityManager::get_instance();
	em->clear();
	em->add_system< blue_sky::ecs::TargetRegistrySystem >( 20 );
	auto* e = em->create_entity();
	e->add_component< core::ecs::TransformComponent >();
	e->add_component< blue_sky::ecs::PerceivableComponent >();
	EXPECT_EQ( 1u, em->get_system< blue_sky::ecs::TargetRegistrySystem >()->debug_entity_count() );
	em->clear();
}

// 段階4: 全 System 追加後に実体生成（シーンと同じ）。
TEST( EnemyAiSceneEcsTest, Stage4_AllSystems )
{
	auto em = core::ecs::EntityManager::get_instance();
	em->clear();
	em->add_system< core::ecs::PhysicsSystem >( 5 );
	em->add_system< blue_sky::ecs::FpsPlayerControlSystem >( 10 );
	em->add_system< blue_sky::ecs::TargetRegistrySystem >( 20 );
	em->add_system< blue_sky::ecs::PerceptionSystem >( 30 );
	em->add_system< blue_sky::ecs::AlertSystem >( 40 );
	em->add_system< blue_sky::ecs::PatrolSystem >( 50 );

	auto* enemy = em->create_entity();
	enemy->add_component< core::ecs::TransformComponent >();
	enemy->add_component< blue_sky::ecs::PerceptionComponent >();
	enemy->add_component< blue_sky::ecs::AlertComponent >();
	enemy->add_component< blue_sky::ecs::PatrolComponent >();

	EXPECT_EQ( 1u, em->get_system< blue_sky::ecs::PatrolSystem >()->debug_entity_count() ) << "enemy -> PatrolSystem";
	em->clear();
}
