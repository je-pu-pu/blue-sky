#include "pch.h"
#include <core/ecs/EntityManager.h>
#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/component/ParticleSystemComponent.h>
#include <core/ecs/system/ParticleSystem.h>
#include <core/ecs/system/ParticleRenderSystem.h>

TEST( EcsTest, EntityManagerTest )
{
	auto entity_manager = core::ecs::EntityManager::get_instance();
	EXPECT_NE( nullptr, entity_manager );

	auto entity = entity_manager->create_entity();
	EXPECT_NE( nullptr, entity );

	auto tc = entity->add_component< core::ecs::TransformComponent >();
	EXPECT_NE( nullptr, tc );

	EXPECT_EQ( tc, entity->get_component< core::ecs::TransformComponent >() );

	entity->remove_component< core::ecs::TransformComponent >();
	EXPECT_EQ( nullptr, entity->get_component< core::ecs::TransformComponent >() );

	auto psc = entity->add_component< core::ecs::ParticleSystemComponent>();
	EXPECT_NE( nullptr, psc );
}

/// @todo ƒŠƒ“ƒN‚·‚éB
/*
TEST( EcsTest, SystemTest )
{
	auto entity_manager = core::ecs::EntityManager::get_instance();
	entity_manager->add_system< core::ecs::ParticleSystem >();
	entity_manager->add_system< core::ecs::ParticleRenderSystem >();

	auto entity = nullptr; // entity_manager->create_entity();

	entity_manager->remove_system< core::ecs::ParticleSystem >();
	entity_manager->remove_system< core::ecs::ParticleRenderSystem >();

	EXPECT_NE( nullptr, entity );
}
*/