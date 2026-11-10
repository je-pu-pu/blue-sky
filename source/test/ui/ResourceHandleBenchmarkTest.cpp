#include "pch.h"

#include <core/ResourceManager.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <unordered_map>
#include <vector>

/**
 * ハンドルを引く手間のベンチマーク（doc/ハンドルの設計.md の 3 章・5 章）
 *
 * 既定では走らない（DISABLED_）。走らせるときは
 *   blue-sky-test.exe --gtest_also_run_disabled_tests --gtest_filter=ResourceHandleBenchmark.*
 * Debug と Release の両方で測る。数字は 1 回引くあたりの時間（ns。何回か測った中央値）。
 *
 * - 生ポインタ : ポインタを 1 回たどる（ハンドルにする前の形）
 * - 番号＋ハッシュの表 : 作り替える前の ResourceManager::resolve と同じ（番号 → リソースの std::unordered_map を 1 回引く）。
 *   作り替えた後も比べられるよう、ここに同じ形を写して残す
 * - ResourceManager::resolve : いま実際に使っている形
 *
 * 順 : 作った順に引く。ばらばら : 混ぜた順に引く（Entity がモデルを指す並びは作った順と揃わない）
 */
namespace
{

struct Payload
{
	int value;
	explicit Payload( int v ) : value( v ) { }
};

/// 作り替える前の形（番号 → リソースのハッシュの表）
class IdTable
{
	std::unordered_map< std::uint64_t, Payload* > by_id_;

public:
	void add( std::uint64_t id, Payload* p ) { by_id_[ id ] = p; }

	Payload* resolve( std::uint64_t id ) const
	{
		auto i = by_id_.find( id );
		return i == by_id_.end() ? nullptr : i->second;
	}
};

volatile long long sink = 0;	///< 最適化で消されないよう、引いた結果をここへ書く

/// lookups 個を引く処理 f を何回か測り、1 回引くあたりの時間（ns）の中央値を返す
template< typename F >
double measure_ns( std::size_t lookups, F f )
{
	const std::size_t rounds_per_sample = std::max< std::size_t >( 1, 2'000'000 / lookups );	// 1 回の計測で 200 万回くらい引く
	std::vector< double > samples;

	f();	// 温める（キャッシュに載せる）

	for ( int s = 0; s < 7; s++ )
	{
		const auto start = std::chrono::steady_clock::now();

		for ( std::size_t r = 0; r < rounds_per_sample; r++ )
		{
			sink = sink + f();
		}

		const auto end = std::chrono::steady_clock::now();
		samples.push_back( std::chrono::duration< double, std::nano >( end - start ).count() / static_cast< double >( rounds_per_sample * lookups ) );
	}

	std::sort( samples.begin(), samples.end() );
	return samples[ samples.size() / 2 ];
}

void run_benchmark( std::size_t count )
{
	using Manager = core::ResourceManager< Payload >;

	Manager manager;
	IdTable table;
	std::vector< Payload* > raw;
	std::vector< Manager::Handle > handles;
	std::vector< std::uint64_t > ids;

	for ( std::size_t i = 0; i < count; i++ )
	{
		Payload* p = manager.create( static_cast< int >( i ) );
		raw.push_back( p );
		handles.push_back( manager.handle_of( p ) );
		ids.push_back( i + 1 );
		table.add( i + 1, p );
	}

	std::vector< std::size_t > order( count );
	std::iota( order.begin(), order.end(), 0 );

	for ( int pattern = 0; pattern < 2; pattern++ )
	{
		if ( pattern == 1 )
		{
			std::shuffle( order.begin(), order.end(), std::mt19937( 1 ) );
		}

		std::vector< Payload* > raw_in_order;
		std::vector< Manager::Handle > handles_in_order;
		std::vector< std::uint64_t > ids_in_order;

		for ( std::size_t i : order )
		{
			raw_in_order.push_back( raw[ i ] );
			handles_in_order.push_back( handles[ i ] );
			ids_in_order.push_back( ids[ i ] );
		}

		const double raw_ns = measure_ns( count, [&] { long long sum = 0; for ( const Payload* p : raw_in_order ) { sum += p->value; } return sum; } );
		const double table_ns = measure_ns( count, [&] { long long sum = 0; for ( std::uint64_t id : ids_in_order ) { if ( const Payload* p = table.resolve( id ) ) { sum += p->value; } } return sum; } );
		const double manager_ns = measure_ns( count, [&] { long long sum = 0; for ( const auto& h : handles_in_order ) { if ( const Payload* p = manager.resolve( h ) ) { sum += p->value; } } return sum; } );

		std::cout << std::fixed << std::setprecision( 2 )
			<< "[ハンドルを引く手間] " << count << " 個・" << ( pattern == 0 ? "順" : "ばらばら" )
			<< " : 生ポインタ " << raw_ns << " ns / 番号＋ハッシュの表 " << table_ns << " ns / ResourceManager::resolve " << manager_ns << " ns"
			<< "（1 回あたり）" << std::endl;
	}
}

} // namespace

TEST( ResourceHandleBenchmark, DISABLED_引く手間_1万個 )
{
	run_benchmark( 10'000 );
}

TEST( ResourceHandleBenchmark, DISABLED_引く手間_10万個 )
{
	run_benchmark( 100'000 );
}
