#include "pch.h"

#include <blue_sky/GameMain.h>

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

TEST( EcsTest, SystemTest )
{
	/**
	 * このテストを実行すると、 entity_manager->add_system< core::ecs::ParticleRenderSystem >(); の中で、
	 * new ParticleRenderSystem が呼ばれ、間接的に GameMain::get_instance() が呼ばれる。その中で Direct3D11 を初期化しようとするが hWnd が nullptr なので落ちる。
	 * よってテストは失敗する。
	 * 
	 * @todo GameMain をモック化するか、 Direct3D11 の初期化をスキップする方法を考える。
	 */

	// blue_sky::GameMain::get_instance();

	auto entity_manager = core::ecs::EntityManager::get_instance();
	EXPECT_NE( nullptr, entity_manager );

	entity_manager->add_system< core::ecs::ParticleSystem >();
	entity_manager->add_system< core::ecs::ParticleRenderSystem >();

	auto entity = entity_manager->create_entity();
	EXPECT_NE( nullptr, entity );

	entity_manager->remove_system< core::ecs::ParticleSystem >();
	entity_manager->remove_system< core::ecs::ParticleRenderSystem >();
}
