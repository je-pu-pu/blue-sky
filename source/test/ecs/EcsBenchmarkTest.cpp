#include "pch.h"

#include <core/ecs/EntityManager.h>
#include <core/ecs/System.h>
#include <core/ecs/SparseIndex.h>
#include <core/ecs/component/TransformComponent.h>

#include <blue_sky/ecs/system/ObjectRenderDataSystem.h>
#include <blue_sky/ecs/system/MoveToTargetSystem.h>
#include <blue_sky/ecs/component/ObjectRenderComponent.h>
#include <blue_sky/ecs/component/TargetMoverComponent.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <random>
#include <unordered_map>
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
	void update() override
	{
		for_each( [] ( ComponentTuple& t )
		{
			auto* p = std::get< BenchPosition* >( t );
			const auto* v = std::get< BenchVelocity* >( t );
			p->x += v->x * 0.016f;
			p->y += v->y * 0.016f;
			p->z += v->z * 0.016f;
		} );
	}
};

/// 3 種類の部品を持つ System（体力を回復し、位置で少し変える）
class BenchHealthSystem : public core::ecs::System< BenchPosition, BenchVelocity, BenchHealth >
{
public:
	void update() override
	{
		for_each( [] ( ComponentTuple& t )
		{
			const auto* p = std::get< BenchPosition* >( t );
			const auto* v = std::get< BenchVelocity* >( t );
			auto* h = std::get< BenchHealth* >( t );
			h->value += h->regen * 0.016f + ( p->x - v->x ) * 0.0f;
		} );
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

namespace {

/**
 * EntityId から添字を引く表の比べ（SparseIndex と unordered_map）
 *
 * 部品の置き場の引き方を決めるために測る。番号は作った順の通し番号（今の EntityId と同じ）で、
 * 足す・作った順に引く・ばらばらの順に引く・消す の時間を出す
 */
template< typename Table, typename Set, typename Find, typename Erase >
void measure_table( const char* name, std::size_t n, Set set, Find find, Erase erase )
{
	constexpr int find_repeat = 10;

	Table table;

	std::vector< core::ecs::EntityId > shuffled( n );
	for ( std::size_t i = 0; i < n; i++ ) { shuffled[ i ] = static_cast< core::ecs::EntityId >( i ); }
	std::shuffle( shuffled.begin(), shuffled.end(), std::mt19937( 1234 ) );

	auto begin = Clock::now();
	for ( std::size_t i = 0; i < n; i++ ) { set( table, static_cast< core::ecs::EntityId >( i ), static_cast< std::uint32_t >( i ) ); }
	const double set_ms = elapsed_ms( begin );

	std::uint64_t sum = 0;
	begin = Clock::now();
	for ( int r = 0; r < find_repeat; r++ ) { for ( std::size_t i = 0; i < n; i++ ) { sum += find( table, static_cast< core::ecs::EntityId >( i ) ); } }
	const double find_seq_ms = elapsed_ms( begin ) / find_repeat;

	begin = Clock::now();
	for ( int r = 0; r < find_repeat; r++ ) { for ( auto id : shuffled ) { sum += find( table, id ); } }
	const double find_rand_ms = elapsed_ms( begin ) / find_repeat;

	begin = Clock::now();
	for ( auto id : shuffled ) { erase( table, id ); }
	const double erase_ms = elapsed_ms( begin );

	EXPECT_GT( sum, 0u );

	const double per = 1000000.0 / static_cast< double >( n );
	std::printf( "  %-14s : 足す %7.1f ns / 作った順に引く %6.1f ns / ばらばらに引く %6.1f ns / 消す %7.1f ns（1 個あたり）\n", name, set_ms * per, find_seq_ms * per, find_rand_ms * per, erase_ms * per );
}

void run_table_benchmark( std::size_t n )
{
	using Map = std::unordered_map< core::ecs::EntityId, std::uint32_t >;

	std::printf( "[ECS 計測] 番号 → 添字 の表 %zu 個\n", n );

	measure_table< core::ecs::SparseIndex >( "SparseIndex", n,
		[] ( core::ecs::SparseIndex& t, core::ecs::EntityId id, std::uint32_t v ) { t.set( id, v ); },
		[] ( const core::ecs::SparseIndex& t, core::ecs::EntityId id ) { return t.find( id ); },
		[] ( core::ecs::SparseIndex& t, core::ecs::EntityId id ) { t.erase( id ); } );

	measure_table< Map >( "unordered_map", n,
		[] ( Map& t, core::ecs::EntityId id, std::uint32_t v ) { t.emplace( id, v ); },
		[] ( const Map& t, core::ecs::EntityId id ) { auto i = t.find( id ); return i == t.end() ? 0u : i->second; },
		[] ( Map& t, core::ecs::EntityId id ) { t.erase( id ); } );

	std::fflush( stdout );
}

} // namespace

TEST( EcsBenchmark, DISABLED_SparseIndexVsUnorderedMap )
{
	run_table_benchmark( 10000 );
	run_table_benchmark( 100000 );
}

namespace {

/**
 * 作った順の一覧を持つ System（ObjectRenderDataSystem・MoveToTargetSystem）で、
 * 物体が 1 つ生まれる・消えるフレームの手間を測る
 *
 * 以前は対象が 1 つ増減するたびに一覧を全部作り直して並べ替えていた（O(n log n)）。
 * N 個ある所で「何も増減しない」「1 つ足す」「1 つ消す」フレームを繰り返し、1 フレームあたりの
 * 描く対象の一覧を読む（get_render_targets）・目標の更新（update_targets）の時間を出す
 */
void run_ordered_list_benchmark( std::size_t n )
{
	constexpr int frames = 200;

	core::ecs::EntityManager entity_manager;
	entity_manager.add_system< blue_sky::ecs::ObjectRenderDataSystem >( 0, nullptr );
	entity_manager.add_system< blue_sky::ecs::MoveToTargetSystem >( 1 );

	auto* render_system = entity_manager.get_system< blue_sky::ecs::ObjectRenderDataSystem >();
	auto* move_system = entity_manager.get_system< blue_sky::ecs::MoveToTargetSystem >();

	std::vector< core::ecs::EntityId > ids;

	auto create = [ & ] ()
	{
		auto* e = entity_manager.create_entity();
		e->add_component< core::ecs::TransformComponent >();
		e->add_component< blue_sky::ecs::ObjectRenderComponent >();
		e->add_component< blue_sky::ecs::TargetMoverComponent >();
		ids.push_back( e->get_id() );
	};

	for ( std::size_t i = 0; i < n; i++ )
	{
		create();
	}

	std::size_t sum = 0;

	// 1 フレームぶん : 描く対象の一覧を読む・目標を更新する
	auto frame = [ & ] ( double& render_ms, double& move_ms )
	{
		auto begin = Clock::now();
		sum += render_system->get_render_targets().size();
		render_ms += elapsed_ms( begin );

		begin = Clock::now();
		move_system->update_targets( 1.f / 60.f );
		move_ms += elapsed_ms( begin );
	};

	double still_render = 0.0, still_move = 0.0;
	double add_render = 0.0, add_move = 0.0;
	double remove_render = 0.0, remove_move = 0.0;

	frame( still_render, still_move );	// 最初の一覧を作る（測らない）
	still_render = still_move = 0.0;

	for ( int f = 0; f < frames; f++ )
	{
		frame( still_render, still_move );
	}

	for ( int f = 0; f < frames; f++ )
	{
		create();
		frame( add_render, add_move );
	}

	std::mt19937 random( 1234 );

	for ( int f = 0; f < frames; f++ )
	{
		// 途中の 1 つを消す
		const std::size_t index = std::uniform_int_distribution< std::size_t >( 0, ids.size() - 1 )( random );
		entity_manager.destroy_entity( ids[ index ] );
		ids.erase( ids.begin() + static_cast< std::ptrdiff_t >( index ) );

		frame( remove_render, remove_move );
	}

	EXPECT_GT( sum, 0u );
	EXPECT_EQ( ids.size(), render_system->get_render_targets().size() );
	EXPECT_EQ( ids.size(), move_system->debug_entity_count() );

	std::printf( "[ECS 計測] 作った順の一覧 Entity %zu 個（1 フレームあたり・%d フレームの平均、ミリ秒）\n", n, frames );
	std::printf( "  増減なし   : 描く対象の一覧 %8.4f / 目標の更新 %8.4f\n", still_render / frames, still_move / frames );
	std::printf( "  1 つ足す   : 描く対象の一覧 %8.4f / 目標の更新 %8.4f\n", add_render / frames, add_move / frames );
	std::printf( "  1 つ消す   : 描く対象の一覧 %8.4f / 目標の更新 %8.4f\n", remove_render / frames, remove_move / frames );
	std::fflush( stdout );
}

} // namespace

TEST( EcsBenchmark, DISABLED_OrderedListAddRemove )
{
	run_ordered_list_benchmark( 10000 );
	run_ordered_list_benchmark( 100000 );
}
