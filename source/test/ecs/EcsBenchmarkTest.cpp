#include "pch.h"

#include <core/ecs/EntityManager.h>
#include <core/ecs/System.h>

#include <chrono>
#include <cstdio>
#include <vector>

/**
 * ECS の手間を測るベンチマーク（既定では走らない）
 *
 * Entity が 1 万を超える前提で、次の手間を 1 万個と 10 万個で測り、標準出力に出す。
 *   - Entity を作り、部品を 3 つ付ける（System は先に登録しておき、付けるたびに System の対象に入る）
 *   - 部品を引く（get_component）
 *   - System が対象を回す（2 種類・3 種類の部品を持つ System の update。1 フレーム相当）
 *   - Entity を消す
 *
 * 走らせ方 : blue-sky-test.exe --gtest_also_run_disabled_tests --gtest_filter=EcsBenchmark.*
 * 部品の持ち方を作り替える前後の比べる基準にする（doc/ECS移行計画.md の 6 節）
 */

namespace {

using Clock = std::chrono::steady_clock;

/// 経過時間（ミリ秒）
double elapsed_ms( Clock::time_point begin )
{
	return std::chrono::duration< double, std::milli >( Clock::now() - begin ).count();
}

struct BenchPosition : public core::ecs::Component
{
	float x = 0.f, y = 0.f, z = 0.f;
};

struct BenchVelocity : public core::ecs::Component
{
	float x = 1.f, y = 2.f, z = 3.f;
};

struct BenchHealth : public core::ecs::Component
{
	float value = 100.f;
	float regen = 0.5f;
};

/// 2 種類の部品を持つ System（位置に速度を足す）
class BenchMoveSystem : public core::ecs::System< BenchPosition, BenchVelocity >
{
public:
	void update( ComponentTuple& t ) const override
	{
		auto* p = std::get< BenchPosition* >( t );
		const auto* v = std::get< BenchVelocity* >( t );
		p->x += v->x * 0.016f;
		p->y += v->y * 0.016f;
		p->z += v->z * 0.016f;
	}
};

/// 3 種類の部品を持つ System（体力を回復し、位置で少し変える）
class BenchHealthSystem : public core::ecs::System< BenchPosition, BenchVelocity, BenchHealth >
{
public:
	void update( ComponentTuple& t ) const override
	{
		const auto* p = std::get< BenchPosition* >( t );
		const auto* v = std::get< BenchVelocity* >( t );
		auto* h = std::get< BenchHealth* >( t );
		h->value += h->regen * 0.016f + ( p->x - v->x ) * 0.0f;
	}
};

/// N 個の Entity で測って出す
void run_benchmark( std::size_t n )
{
	constexpr int get_repeat = 10;		///< 部品を引くのを全 Entity について繰り返す回数
	constexpr int update_repeat = 30;	///< System の update を回すフレーム数

	core::ecs::EntityManager entity_manager;
	entity_manager.add_system< BenchMoveSystem >( 0 );
	entity_manager.add_system< BenchHealthSystem >( 1 );

	std::vector< core::ecs::Entity* > entities;
	entities.reserve( n );

	// 作る・付ける
	auto begin = Clock::now();

	for ( std::size_t i = 0; i < n; i++ )
	{
		auto* e = entity_manager.create_entity();
		e->add_component< BenchPosition >();
		e->add_component< BenchVelocity >();
		e->add_component< BenchHealth >();
		entities.push_back( e );
	}

	const double create_ms = elapsed_ms( begin );

	EXPECT_EQ( n, entity_manager.get_system< BenchMoveSystem >()->debug_entity_count() );
	EXPECT_EQ( n, entity_manager.get_system< BenchHealthSystem >()->debug_entity_count() );

	// 部品を引く（3 種類を順に）
	begin = Clock::now();
	float sum = 0.f;

	for ( int r = 0; r < get_repeat; r++ )
	{
		for ( auto* e : entities )
		{
			sum += e->get_component< BenchPosition >()->x;
			sum += e->get_component< BenchVelocity >()->y;
			sum += e->get_component< BenchHealth >()->value;
		}
	}

	const double get_ms = elapsed_ms( begin ) / get_repeat;
	EXPECT_GT( sum, 0.f );

	// System が回す（1 フレーム = 全 System の update 1 回）
	begin = Clock::now();

	for ( int r = 0; r < update_repeat; r++ )
	{
		entity_manager.update();
	}

	const double update_ms = elapsed_ms( begin ) / update_repeat;

	// 消す
	begin = Clock::now();

	for ( auto* e : entities )
	{
		entity_manager.destroy_entity( e->get_id() );
	}

	const double destroy_ms = elapsed_ms( begin );

	EXPECT_EQ( 0u, entity_manager.get_system< BenchMoveSystem >()->debug_entity_count() );

	const double per = 1000000.0 / static_cast< double >( n );	// ミリ秒 → 1 個あたりのナノ秒

	std::printf( "[ECS 計測] Entity %zu 個\n", n );
	std::printf( "  作る＋部品 3 つ付ける : 全体 %10.3f ms / 1 個 %8.1f ns\n", create_ms, create_ms * per );
	std::printf( "  部品を引く（3 種類）  : 全体 %10.3f ms / 1 個 %8.1f ns（%d 回の平均）\n", get_ms, get_ms * per, get_repeat );
	std::printf( "  System の update      : 1 フレーム %8.3f ms / 1 個 %8.1f ns（System 2 つ・%d フレームの平均）\n", update_ms, update_ms * per, update_repeat );
	std::printf( "  Entity を消す         : 全体 %10.3f ms / 1 個 %8.1f ns\n", destroy_ms, destroy_ms * per );
	std::fflush( stdout );
}

} // namespace

TEST( EcsBenchmark, DISABLED_Entities10k )
{
	run_benchmark( 10000 );
}

TEST( EcsBenchmark, DISABLED_Entities100k )
{
	run_benchmark( 100000 );
}
