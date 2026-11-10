#include "pch.h"

#include <blue_sky/ecs/system/MoveToTargetSystem.h>
#include <blue_sky/ecs/component/TargetMoverComponent.h>
#include <blue_sky/ecs/component/TargetLocationComponent.h>
#include <blue_sky/ecs/component/TargetDirectionComponent.h>

#include <core/ecs/EntityManager.h>

#include <functional>
#include <vector>

using blue_sky::ecs::MoveToTargetSystem;
using blue_sky::ecs::TargetMover;
using blue_sky::ecs::TargetMoverComponent;
using blue_sky::ecs::TargetLocationComponent;
using blue_sky::ecs::TargetDirectionComponent;
using core::ecs::EntityManager;
using core::Vector;

namespace
{

/// 剛体なしで動きを記録する偽物（ActiveObject の代わり）
class FakeMover : public TargetMover
{
public:
	Vector location = Vector( 0.f, 0.f, 0.f );

	int move_count = 0;					///< 速度を向けた回数
	Vector last_move_target;
	float_t last_move_speed = 0.f;

	int stop_count = 0;					///< 止めた回数
	int arrive_count = 0;				///< 着いたと知らされた回数
	bool stopped_before_arrive = false;	///< 知らせる前に止めていたか

	int chase_degree_count = 0;
	float_t last_chase_degree = 0.f;

	int chase_to_count = 0;
	Vector last_chase_to;

	std::vector< int >* arrive_order = nullptr;	///< 着いた順を記録する先（無ければ記録しない）
	int name = 0;

	std::function< void () > on_arrive;		///< 着いたときに追加でする事（テストで使う）

	Vector get_target_mover_location() const override { return location; }

	void move_toward_target_location( const Vector& target_location, float_t speed ) override
	{
		move_count++;
		last_move_target = target_location;
		last_move_speed = speed;
	}

	void stop_at_target_location( const Vector& target_location ) override
	{
		stop_count++;
		location = target_location;
	}

	void notify_arrive_at_target_location() override
	{
		stopped_before_arrive = stop_count > 0;
		arrive_count++;

		if ( arrive_order )
		{
			arrive_order->push_back( name );
		}

		if ( on_arrive )
		{
			on_arrive();
		}
	}

	void chase_target_direction_degree( float_t degree, float_t ) override
	{
		chase_degree_count++;
		last_chase_degree = degree;
	}

	void chase_target_direction_to( const Vector& location, float_t ) override
	{
		chase_to_count++;
		last_chase_to = location;
	}
};

/// 動かす窓口を付けた Entity を作る
core::ecs::EntityId create_mover_entity( EntityManager& manager, FakeMover& mover )
{
	auto* entity = manager.create_entity();
	entity->add_component< TargetMoverComponent >()->mover = &mover;
	return entity->get_id();
}

MoveToTargetSystem* add_system( EntityManager& manager )
{
	manager.add_system< MoveToTargetSystem >( 0 );
	return manager.get_system< MoveToTargetSystem >();
}

} // namespace

// ---- 着いたかの判定（以前は ActiveObjectManager::is_target_location_reached） ----

TEST( MoveToTargetSystemTest, 残りが_0_1_未満なら着いたことにする )
{
	EXPECT_TRUE( MoveToTargetSystem::is_target_location_reached( 0.05f, 1.f, 1.f / 60.f ) );
	EXPECT_FALSE( MoveToTargetSystem::is_target_location_reached( 0.5f, 1.f, 1.f / 60.f ) );
}

TEST( MoveToTargetSystemTest, このフレームで進める距離以内なら着いたことにする )
{
	// 速さ 30 で 1/30 秒のフレームなら 1 進む。以前は 0.1 未満に入るまで目標の周りを行ったり来たりしていた
	EXPECT_TRUE( MoveToTargetSystem::is_target_location_reached( 0.8f, 30.f, 1.f / 30.f ) );
	EXPECT_FALSE( MoveToTargetSystem::is_target_location_reached( 1.2f, 30.f, 1.f / 30.f ) );
}

TEST( MoveToTargetSystemTest, 経過時間が_0_でも_0_1_の判定は効く )
{
	EXPECT_TRUE( MoveToTargetSystem::is_target_location_reached( 0.09f, 10.f, 0.f ) );
	EXPECT_FALSE( MoveToTargetSystem::is_target_location_reached( 0.2f, 10.f, 0.f ) );
}

// ---- 目標地点 ----

TEST( MoveToTargetSystemTest, 着くまでは毎フレーム目標へ速度を向ける )
{
	EntityManager manager;
	auto* system = add_system( manager );

	FakeMover mover;
	const auto id = create_mover_entity( manager, mover );

	ASSERT_TRUE( system->set_target_location( id, Vector( 10.f, 0.f, 0.f ), 2.f ) );

	system->update_targets( 1.f / 60.f );
	system->update_targets( 1.f / 60.f );

	EXPECT_EQ( mover.move_count, 2 );
	EXPECT_FLOAT_EQ( mover.last_move_target.x(), 10.f );
	EXPECT_FLOAT_EQ( mover.last_move_speed, 2.f );
	EXPECT_EQ( mover.arrive_count, 0 );
	EXPECT_EQ( mover.stop_count, 0 );
}

TEST( MoveToTargetSystemTest, 着いたら目標地点に置いて止め_一度だけ知らせて目標を外す )
{
	EntityManager manager;
	auto* system = add_system( manager );

	FakeMover mover;
	const auto id = create_mover_entity( manager, mover );

	ASSERT_TRUE( system->set_target_location( id, Vector( 10.f, 0.f, 0.f ), 2.f ) );

	mover.location = Vector( 9.95f, 0.f, 0.f );		// 残り 0.05（0.1 未満）
	system->update_targets( 1.f / 60.f );

	EXPECT_EQ( mover.stop_count, 1 );
	EXPECT_EQ( mover.arrive_count, 1 );
	EXPECT_TRUE( mover.stopped_before_arrive );
	EXPECT_FLOAT_EQ( mover.location.x(), 10.f );
	EXPECT_EQ( mover.move_count, 0 );
	EXPECT_EQ( manager.find( id )->get_component< TargetLocationComponent >(), nullptr );

	// 次のフレームからは何もしない（二度知らせない）
	system->update_targets( 1.f / 60.f );
	system->update_targets( 1.f / 60.f );

	EXPECT_EQ( mover.arrive_count, 1 );
	EXPECT_EQ( mover.stop_count, 1 );
	EXPECT_EQ( mover.move_count, 0 );
}

TEST( MoveToTargetSystemTest, 目標地点を設定し直すと置き換わる )
{
	EntityManager manager;
	auto* system = add_system( manager );

	FakeMover mover;
	const auto id = create_mover_entity( manager, mover );

	ASSERT_TRUE( system->set_target_location( id, Vector( 10.f, 0.f, 0.f ), 2.f ) );
	ASSERT_TRUE( system->set_target_location( id, Vector( 0.f, 5.f, 0.f ), 3.f ) );

	system->update_targets( 1.f / 60.f );

	EXPECT_EQ( mover.move_count, 1 );
	EXPECT_FLOAT_EQ( mover.last_move_target.y(), 5.f );
	EXPECT_FLOAT_EQ( mover.last_move_speed, 3.f );
}

TEST( MoveToTargetSystemTest, 着いたと知らせた先で設定した新しい目標は残る )
{
	EntityManager manager;
	auto* system = add_system( manager );

	FakeMover mover;
	const auto id = create_mover_entity( manager, mover );

	ASSERT_TRUE( system->set_target_location( id, Vector( 0.f, 0.f, 0.f ), 1.f ) );		// 今いる所 → すぐ着く

	mover.on_arrive = [&] () { system->set_target_location( id, Vector( 0.f, 0.f, 20.f ), 1.f ); mover.on_arrive = nullptr; };
	system->update_targets( 1.f / 60.f );

	EXPECT_EQ( mover.arrive_count, 1 );
	ASSERT_NE( manager.find( id )->get_component< TargetLocationComponent >(), nullptr );

	system->update_targets( 1.f / 60.f );

	EXPECT_EQ( mover.move_count, 1 );
	EXPECT_FLOAT_EQ( mover.last_move_target.z(), 20.f );
}

TEST( MoveToTargetSystemTest, 同じフレームに着いた物体は作った順に知らせる )
{
	EntityManager manager;
	auto* system = add_system( manager );

	std::vector< int > order;
	FakeMover movers[ 5 ];
	std::vector< core::ecs::EntityId > ids;

	for ( int n = 0; n < 5; n++ )
	{
		movers[ n ].name = n;
		movers[ n ].arrive_order = &order;
		ids.push_back( create_mover_entity( manager, movers[ n ] ) );
	}

	// 設定する順は作った順と逆にする
	for ( int n = 4; n >= 0; n-- )
	{
		ASSERT_TRUE( system->set_target_location( ids[ n ], Vector( 0.f, 0.f, 0.f ), 1.f ) );
	}

	system->update_targets( 1.f / 60.f );

	EXPECT_EQ( order, ( std::vector< int >{ 0, 1, 2, 3, 4 } ) );
}

TEST( MoveToTargetSystemTest, 消した_Entity_の目標は捨てる )
{
	EntityManager manager;
	auto* system = add_system( manager );

	FakeMover mover;
	FakeMover other;
	const auto id = create_mover_entity( manager, mover );
	const auto other_id = create_mover_entity( manager, other );

	ASSERT_TRUE( system->set_target_location( id, Vector( 10.f, 0.f, 0.f ), 1.f ) );
	ASSERT_TRUE( system->set_target_location( other_id, Vector( 10.f, 0.f, 0.f ), 1.f ) );

	manager.destroy_entity( id );
	system->update_targets( 1.f / 60.f );

	EXPECT_EQ( mover.move_count, 0 );
	EXPECT_EQ( other.move_count, 1 );
	EXPECT_EQ( system->debug_entity_count(), 1u );

	// 消した Entity には設定できない
	EXPECT_FALSE( system->set_target_location( id, Vector( 0.f, 0.f, 0.f ), 1.f ) );
}

TEST( MoveToTargetSystemTest, 窓口の無い_Entity_には目標を設定できない )
{
	EntityManager manager;
	auto* system = add_system( manager );

	const auto id = manager.create_entity()->get_id();

	EXPECT_FALSE( system->set_target_location( id, Vector( 0.f, 0.f, 0.f ), 1.f ) );
	EXPECT_FALSE( system->set_target_direction( id, 90.f, 1.f ) );
	EXPECT_FALSE( system->set_target_direction_entity( id, id, 1.f ) );
}

// ---- 目標の向き ----

TEST( MoveToTargetSystemTest, 決まった向きは着いても外さず追い続ける )
{
	EntityManager manager;
	auto* system = add_system( manager );

	FakeMover mover;
	const auto id = create_mover_entity( manager, mover );

	ASSERT_TRUE( system->set_target_direction( id, 90.f, 1.f ) );

	system->update_targets( 1.f / 60.f );
	system->update_targets( 1.f / 60.f );

	EXPECT_EQ( mover.chase_degree_count, 2 );
	EXPECT_FLOAT_EQ( mover.last_chase_degree, 90.f );
	EXPECT_EQ( mover.chase_to_count, 0 );
}

TEST( MoveToTargetSystemTest, 物体の方向を追わせると決まった向きはやめる_逆も同じ )
{
	EntityManager manager;
	auto* system = add_system( manager );

	FakeMover mover;
	FakeMover target;
	target.location = Vector( 3.f, 0.f, 4.f );

	const auto id = create_mover_entity( manager, mover );
	const auto target_id = create_mover_entity( manager, target );

	ASSERT_TRUE( system->set_target_direction( id, 90.f, 1.f ) );
	ASSERT_TRUE( system->set_target_direction_entity( id, target_id, 1.f ) );

	system->update_targets( 1.f / 60.f );

	EXPECT_EQ( mover.chase_degree_count, 0 );
	EXPECT_EQ( mover.chase_to_count, 1 );
	EXPECT_FLOAT_EQ( mover.last_chase_to.x(), 3.f );
	EXPECT_FLOAT_EQ( mover.last_chase_to.z(), 4.f );

	ASSERT_TRUE( system->set_target_direction( id, 45.f, 1.f ) );

	system->update_targets( 1.f / 60.f );

	EXPECT_EQ( mover.chase_degree_count, 1 );
	EXPECT_EQ( mover.chase_to_count, 1 );
}

TEST( MoveToTargetSystemTest, 向く先の物体が消えたら目標の向きを捨てる )
{
	EntityManager manager;
	auto* system = add_system( manager );

	FakeMover mover;
	FakeMover target;

	const auto id = create_mover_entity( manager, mover );
	const auto target_id = create_mover_entity( manager, target );

	ASSERT_TRUE( system->set_target_direction_entity( id, target_id, 1.f ) );

	manager.destroy_entity( target_id );
	system->update_targets( 1.f / 60.f );

	EXPECT_EQ( mover.chase_to_count, 0 );
	EXPECT_EQ( manager.find( id )->get_component< TargetDirectionComponent >(), nullptr );
}

TEST( MoveToTargetSystemTest, やり直すと全部の目標を捨てる )
{
	EntityManager manager;
	auto* system = add_system( manager );

	FakeMover mover;
	const auto id = create_mover_entity( manager, mover );

	ASSERT_TRUE( system->set_target_location( id, Vector( 10.f, 0.f, 0.f ), 1.f ) );
	ASSERT_TRUE( system->set_target_direction( id, 90.f, 1.f ) );

	system->clear_targets();
	system->update_targets( 1.f / 60.f );

	EXPECT_EQ( mover.move_count, 0 );
	EXPECT_EQ( mover.chase_degree_count, 0 );
	EXPECT_EQ( manager.find( id )->get_component< TargetLocationComponent >(), nullptr );
	EXPECT_EQ( manager.find( id )->get_component< TargetDirectionComponent >(), nullptr );
}

TEST( MoveToTargetSystemTest, EntityManager_の_update_では動かさない )
{
	// EntityManager::update（ほかのシーンが呼ぶ）で二重に進めないように、update_targets だけが動かす
	EntityManager manager;
	auto* system = add_system( manager );

	FakeMover mover;
	const auto id = create_mover_entity( manager, mover );

	ASSERT_TRUE( system->set_target_location( id, Vector( 10.f, 0.f, 0.f ), 1.f ) );

	manager.update();

	EXPECT_EQ( mover.move_count, 0 );
}
