#include "pch.h"

#include <core/ecs/EntityManager.h>
#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/component/RigidBodyComponent.h>
#include <core/ecs/component/BalloonComponent.h>
#include <core/ecs/component/ParticleSystemComponent.h>
#include <core/ecs/system/ParticleSystem.h>
#include <core/ecs/system/PhysicsSystem.h>

namespace {

/**
 * update() が呼ばれた回数を記録するテスト用 System
 *
 * TransformComponent のみを要求する。
 * EntityManager::update() を呼ぶと、登録済みの Entity 1 つにつき
 * update_count が 1 増える。これにより System への自動登録・解除を検証できる。
 */
class CountSystem : public core::ecs::System< core::ecs::TransformComponent >
{
public:
	mutable int update_count = 0;
	void update( ComponentTuple& ) const override { update_count++; }
};

/**
 * 複数コンポーネントを要求するテスト用 System
 *
 * TransformComponent と RigidBodyComponent の両方が揃った Entity だけが
 * 自動登録される。片方だけでは登録されないことを検証するために使用する。
 */
class DualCountSystem : public core::ecs::System< core::ecs::TransformComponent, core::ecs::RigidBodyComponent >
{
public:
	mutable int update_count = 0;
	void update( ComponentTuple& ) const override { update_count++; }
};

} // namespace

// ---------------------------------------------------------------------------
// Entity 基本操作
// ---------------------------------------------------------------------------

/**
 * Entity の生成と Component の追加・取得・削除の基本フローを検証する
 */
TEST( EcsTest, EntityManagerTest )
{
	auto em = core::ecs::EntityManager::get_instance();
	em->clear();

	auto entity = em->create_entity();
	EXPECT_NE( nullptr, entity );

	// Component を追加し、同じポインタで取得できることを確認
	auto tc = entity->add_component< core::ecs::TransformComponent >();
	EXPECT_NE( nullptr, tc );
	EXPECT_EQ( tc, entity->get_component< core::ecs::TransformComponent >() );

	// 削除後は nullptr が返る
	entity->remove_component< core::ecs::TransformComponent >();
	EXPECT_EQ( nullptr, entity->get_component< core::ecs::TransformComponent >() );

	// 別の型の Component も追加できる
	auto psc = entity->add_component< core::ecs::ParticleSystemComponent >();
	EXPECT_NE( nullptr, psc );
}

/**
 * create_entity() が毎回異なる ID を付与することを検証する
 *
 * EntityManager は内部カウンタ (next_entity_id_) をインクリメントするため、
 * 同一セッション内で ID が重複することはない。
 */
TEST( EcsTest, EntityIdUniqueness )
{
	auto em = core::ecs::EntityManager::get_instance();
	em->clear();

	auto e1 = em->create_entity();
	auto e2 = em->create_entity();
	auto e3 = em->create_entity();

	EXPECT_NE( e1->get_id(), e2->get_id() );
	EXPECT_NE( e2->get_id(), e3->get_id() );
	EXPECT_NE( e1->get_id(), e3->get_id() );
}

/**
 * destroy_entity() で Entity が破棄されることを検証する
 *
 * 破棄された ID は再利用されず、次に作成される Entity は新しい ID を持つ。
 */
TEST( EcsTest, DestroyEntity )
{
	auto em = core::ecs::EntityManager::get_instance();
	em->clear();

	auto entity = em->create_entity();
	auto id = entity->get_id();

	entity->add_component< core::ecs::TransformComponent >();
	em->destroy_entity( id );

	// 破棄後に作成した Entity は別の ID を持つ
	auto entity2 = em->create_entity();
	EXPECT_NE( id, entity2->get_id() );
}

// ---------------------------------------------------------------------------
// Component 操作
// ---------------------------------------------------------------------------

/**
 * 同じ型の Component を同一 Entity に二重追加すると nullptr が返ることを検証する
 *
 * EntityManager::add_component() は、既に同じ型が登録済みの場合に nullptr を返す。
 * 最初に追加したインスタンスはそのまま有効。
 */
TEST( EcsTest, DuplicateComponentReturnsNull )
{
	auto em = core::ecs::EntityManager::get_instance();
	em->clear();

	auto entity = em->create_entity();
	auto first = entity->add_component< core::ecs::TransformComponent >();
	EXPECT_NE( nullptr, first );

	auto second = entity->add_component< core::ecs::TransformComponent >();
	EXPECT_EQ( nullptr, second );

	EXPECT_EQ( first, entity->get_component< core::ecs::TransformComponent >() );
}

/**
 * 追加していない型の Component を取得すると nullptr が返ることを検証する
 */
TEST( EcsTest, GetNonExistentComponent )
{
	auto em = core::ecs::EntityManager::get_instance();
	em->clear();

	auto entity = em->create_entity();
	EXPECT_EQ( nullptr, entity->get_component< core::ecs::TransformComponent >() );
}

/**
 * 1 つの Entity に複数の型の Component を同時に保持できることを検証する
 *
 * また、1 つの Component を削除しても他の Component には影響しないことを確認する。
 */
TEST( EcsTest, MultipleComponentTypes )
{
	auto em = core::ecs::EntityManager::get_instance();
	em->clear();

	auto entity = em->create_entity();
	auto tc = entity->add_component< core::ecs::TransformComponent >();
	auto rb = entity->add_component< core::ecs::RigidBodyComponent >();
	auto bc = entity->add_component< core::ecs::BalloonComponent >();

	EXPECT_NE( nullptr, tc );
	EXPECT_NE( nullptr, rb );
	EXPECT_NE( nullptr, bc );

	EXPECT_EQ( tc, entity->get_component< core::ecs::TransformComponent >() );
	EXPECT_EQ( rb, entity->get_component< core::ecs::RigidBodyComponent >() );
	EXPECT_EQ( bc, entity->get_component< core::ecs::BalloonComponent >() );

	// RigidBodyComponent だけ削除 → 他は残る
	entity->remove_component< core::ecs::RigidBodyComponent >();
	EXPECT_EQ( nullptr, entity->get_component< core::ecs::RigidBodyComponent >() );
	EXPECT_EQ( tc, entity->get_component< core::ecs::TransformComponent >() );
	EXPECT_EQ( bc, entity->get_component< core::ecs::BalloonComponent >() );
}

// ---------------------------------------------------------------------------
// System 管理
// ---------------------------------------------------------------------------

/**
 * 実在する D3D11 非依存の System (ParticleSystem, PhysicsSystem) を
 * 追加・削除できることを検証する
 */
TEST( EcsTest, SystemTest )
{
	auto em = core::ecs::EntityManager::get_instance();
	em->clear();

	em->add_system< core::ecs::ParticleSystem >();
	em->add_system< core::ecs::PhysicsSystem >();

	auto entity = em->create_entity();
	EXPECT_NE( nullptr, entity );

	em->remove_system< core::ecs::ParticleSystem >();
	em->remove_system< core::ecs::PhysicsSystem >();
}

/**
 * get_system() で System のポインタを取得できることを検証する
 *
 * 未追加の場合は nullptr、追加後は有効なポインタ、削除後は再び nullptr。
 */
TEST( EcsTest, GetSystem )
{
	auto em = core::ecs::EntityManager::get_instance();
	em->clear();

	EXPECT_EQ( nullptr, em->get_system< CountSystem >() );

	em->add_system< CountSystem >();
	EXPECT_NE( nullptr, em->get_system< CountSystem >() );

	em->remove_system< CountSystem >();
	EXPECT_EQ( nullptr, em->get_system< CountSystem >() );
}

/**
 * 同じ型の System を二重追加しても無視されることを検証する
 *
 * EntityManager::add_system() は system_map_ に既に同じ型が存在する場合、
 * 何もせずに return する。ポインタが変わらないことで確認。
 */
TEST( EcsTest, DuplicateAddSystemIsNoop )
{
	auto em = core::ecs::EntityManager::get_instance();
	em->clear();

	em->add_system< CountSystem >();
	auto* first = em->get_system< CountSystem >();

	em->add_system< CountSystem >();
	EXPECT_EQ( first, em->get_system< CountSystem >() );

	em->remove_system< CountSystem >();
}

// ---------------------------------------------------------------------------
// System と Component の自動登録
//
// ECS の核心機能。Entity に Component を追加すると、その Component を
// 要求する System に Entity が自動登録され、update() の対象になる。
// ---------------------------------------------------------------------------

/**
 * Entity に Component を追加すると、対応する System に自動登録されることを検証する
 *
 * CountSystem は TransformComponent を要求する。
 * TransformComponent を持つ Entity を作成して update() を呼ぶと、
 * CountSystem::update(ComponentTuple&) が 1 回呼ばれる。
 */
TEST( EcsTest, AutoRegistration )
{
	auto em = core::ecs::EntityManager::get_instance();
	em->clear();

	em->add_system< CountSystem >();

	auto entity = em->create_entity();
	entity->add_component< core::ecs::TransformComponent >();

	auto* system = em->get_system< CountSystem >();
	system->update_count = 0;

	em->update();
	EXPECT_EQ( 1, system->update_count );

	em->remove_system< CountSystem >();
}

/**
 * 複数の Entity がそれぞれ独立して System に登録されることを検証する
 *
 * 3 つの Entity に TransformComponent を追加すると、
 * update() で CountSystem が 3 回呼ばれる。
 */
TEST( EcsTest, AutoRegistrationMultipleEntities )
{
	auto em = core::ecs::EntityManager::get_instance();
	em->clear();

	em->add_system< CountSystem >();

	auto e1 = em->create_entity();
	auto e2 = em->create_entity();
	auto e3 = em->create_entity();

	e1->add_component< core::ecs::TransformComponent >();
	e2->add_component< core::ecs::TransformComponent >();
	e3->add_component< core::ecs::TransformComponent >();

	auto* system = em->get_system< CountSystem >();
	system->update_count = 0;

	em->update();
	EXPECT_EQ( 3, system->update_count );

	em->remove_system< CountSystem >();
}

/**
 * 複数コンポーネントを要求する System は、全てが揃うまで登録されないことを検証する
 *
 * DualCountSystem は TransformComponent + RigidBodyComponent の両方を要求する。
 * TransformComponent だけの状態では update_count は 0 のまま。
 * RigidBodyComponent を追加して全コンポーネントが揃うと 1 になる。
 */
TEST( EcsTest, AutoRegistrationRequiresAllComponents )
{
	auto em = core::ecs::EntityManager::get_instance();
	em->clear();

	em->add_system< DualCountSystem >();

	auto entity = em->create_entity();
	auto* system = em->get_system< DualCountSystem >();

	// 片方だけ → 未登録
	entity->add_component< core::ecs::TransformComponent >();
	system->update_count = 0;
	em->update();
	EXPECT_EQ( 0, system->update_count );

	// 両方揃う → 自動登録
	entity->add_component< core::ecs::RigidBodyComponent >();
	system->update_count = 0;
	em->update();
	EXPECT_EQ( 1, system->update_count );

	em->remove_system< DualCountSystem >();
}

/**
 * Component を削除すると System から自動的に登録解除されることを検証する
 *
 * remove_component() は全 System に on_remove_component() を通知し、
 * System は該当 Entity を component_list_ から削除する。
 */
TEST( EcsTest, RemoveComponentUnregistersFromSystem )
{
	auto em = core::ecs::EntityManager::get_instance();
	em->clear();

	em->add_system< CountSystem >();

	auto entity = em->create_entity();
	entity->add_component< core::ecs::TransformComponent >();

	auto* system = em->get_system< CountSystem >();
	system->update_count = 0;
	em->update();
	EXPECT_EQ( 1, system->update_count );

	// Component を削除 → System から登録解除、update() が呼ばれなくなる
	entity->remove_component< core::ecs::TransformComponent >();
	system->update_count = 0;
	em->update();
	EXPECT_EQ( 0, system->update_count );

	em->remove_system< CountSystem >();
}

/**
 * Entity + Component が先に存在する状態で System を後から追加した場合にも
 * 既存 Entity が自動登録されることを検証する
 *
 * add_system() は追加時に全 Entity を走査し、
 * 必要な Component が揃っている Entity を component_list_ に登録する。
 */
TEST( EcsTest, LateSystemRegistration )
{
	auto em = core::ecs::EntityManager::get_instance();
	em->clear();

	// Entity + Component を先に作成
	auto entity = em->create_entity();
	entity->add_component< core::ecs::TransformComponent >();

	// System を後から追加
	em->add_system< CountSystem >();

	auto* system = em->get_system< CountSystem >();
	system->update_count = 0;
	em->update();
	EXPECT_EQ( 1, system->update_count );

	em->remove_system< CountSystem >();
}

// ---------------------------------------------------------------------------
// EntityManager::clear
// ---------------------------------------------------------------------------

/**
 * clear() で全 Entity, Component, System が破棄されることを検証する
 *
 * clear() 後は get_system() が nullptr を返し、
 * 新しい Entity は作成可能（内部 ID カウンタはリセットされない）。
 */
TEST( EcsTest, Clear )
{
	auto em = core::ecs::EntityManager::get_instance();
	em->clear();

	em->add_system< CountSystem >();

	auto entity = em->create_entity();
	entity->add_component< core::ecs::TransformComponent >();

	em->clear();

	EXPECT_EQ( nullptr, em->get_system< CountSystem >() );

	// clear() 後も新規 Entity を作成可能
	auto entity2 = em->create_entity();
	EXPECT_NE( nullptr, entity2 );
}
