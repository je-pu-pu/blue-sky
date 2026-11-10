#include "pch.h"

#include <core/ecs/SparseIndex.h>
#include <core/ecs/StablePool.h>

#include <memory>
#include <vector>

/**
 * 部品の置き場（StablePool）と、EntityId から添字を引く表（SparseIndex）のテスト
 */

namespace {

using core::ecs::EntityId;
using core::ecs::SparseIndex;
using core::ecs::StablePool;

/// 作った数・壊した数を数える値
struct Counted
{
	static inline int alive = 0;

	int value;

	explicit Counted( int v = 0 ) : value( v ) { alive++; }
	~Counted() { alive--; }

	Counted( const Counted& ) = delete;
	Counted& operator = ( const Counted& ) = delete;
};

/// ムーブだけできる値（unique_ptr を持つ部品の代わり）
struct MoveOnly
{
	std::unique_ptr< int > p = std::make_unique< int >( 5 );
};

/// 1 ページに 1 つしか入らない大きな値（粒子の部品の代わり）
struct Huge
{
	char data[ 40 * 1024 ] = {};
	int value = 0;
};

} // namespace

// ---- SparseIndex ----

TEST( SparseIndexTest, SetFindErase )
{
	SparseIndex index;

	EXPECT_EQ( SparseIndex::INVALID, index.find( 0 ) );
	EXPECT_FALSE( index.contains( 123 ) );

	index.set( 3, 30 );
	index.set( 5000, 50 );

	EXPECT_EQ( 30u, index.find( 3 ) );
	EXPECT_EQ( 50u, index.find( 5000 ) );
	EXPECT_EQ( SparseIndex::INVALID, index.find( 4 ) );
	EXPECT_EQ( 2u, index.size() );

	// 置き換えても数は増えない
	index.set( 3, 31 );
	EXPECT_EQ( 31u, index.find( 3 ) );
	EXPECT_EQ( 2u, index.size() );

	EXPECT_TRUE( index.erase( 3 ) );
	EXPECT_FALSE( index.erase( 3 ) );
	EXPECT_FALSE( index.erase( 999999 ) );
	EXPECT_EQ( SparseIndex::INVALID, index.find( 3 ) );
	EXPECT_EQ( 1u, index.size() );
}

/// ページの番号が全部消えたらページを放す（番号が増え続けても、古い範囲の領域が残らない）
TEST( SparseIndexTest, PageIsReleasedWhenEmpty )
{
	SparseIndex index;

	for ( EntityId id = 0; id < SparseIndex::PAGE_SIZE * 3; id++ )
	{
		index.set( id, id );
	}

	EXPECT_EQ( 3u, index.debug_page_count() );

	// 最初のページの番号を全部消す
	for ( EntityId id = 0; id < SparseIndex::PAGE_SIZE; id++ )
	{
		index.erase( id );
	}

	EXPECT_EQ( 2u, index.debug_page_count() );
	EXPECT_EQ( SparseIndex::PAGE_SIZE * 2, index.size() );
	EXPECT_EQ( static_cast< SparseIndex::Index >( SparseIndex::PAGE_SIZE ), index.find( static_cast< EntityId >( SparseIndex::PAGE_SIZE ) ) );

	index.clear();
	EXPECT_EQ( 0u, index.debug_page_count() );
	EXPECT_TRUE( index.empty() );
}

// ---- StablePool ----

TEST( StablePoolTest, EmplaceFindErase )
{
	StablePool< Counted > pool;

	auto* a = pool.emplace( 10, 1 );
	auto* b = pool.emplace( 20, 2 );

	ASSERT_NE( nullptr, a );
	ASSERT_NE( nullptr, b );
	EXPECT_EQ( a, pool.find( 10 ) );
	EXPECT_EQ( b, pool.find( 20 ) );
	EXPECT_EQ( nullptr, pool.find( 30 ) );
	EXPECT_EQ( 2u, pool.size() );
	EXPECT_EQ( 2, Counted::alive );

	// 同じ id に二度は置かない
	EXPECT_EQ( nullptr, pool.emplace( 10, 9 ) );
	EXPECT_EQ( 1, pool.find( 10 )->value );
	EXPECT_EQ( 2, Counted::alive );

	EXPECT_TRUE( pool.erase( 10 ) );
	EXPECT_FALSE( pool.erase( 10 ) );
	EXPECT_EQ( nullptr, pool.find( 10 ) );
	EXPECT_EQ( 1, Counted::alive );
	EXPECT_EQ( 2, pool.find( 20 )->value );

	pool.clear();
	EXPECT_EQ( 0, Counted::alive );
	EXPECT_TRUE( pool.empty() );
}

/// 置いた値の場所は、ほかの値を足しても（ページが増えても）・消しても動かない
TEST( StablePoolTest, PointersStayStableAcrossGrowthAndErase )
{
	StablePool< Counted > pool;
	const std::size_t n = StablePool< Counted >::PAGE_CAPACITY * 4 + 3;

	std::vector< Counted* > pointers;

	for ( std::size_t i = 0; i < n; i++ )
	{
		pointers.push_back( pool.emplace( static_cast< EntityId >( i ), static_cast< int >( i ) ) );
	}

	// 偶数番を消し、新しい値を足す
	for ( std::size_t i = 0; i < n; i += 2 )
	{
		pool.erase( static_cast< EntityId >( i ) );
	}

	for ( std::size_t i = 0; i < n; i++ )
	{
		pool.emplace( static_cast< EntityId >( n + i ), -1 );
	}

	for ( std::size_t i = 1; i < n; i += 2 )
	{
		EXPECT_EQ( pointers[ i ], pool.find( static_cast< EntityId >( i ) ) );
		EXPECT_EQ( static_cast< int >( i ), pointers[ i ]->value );
	}

	EXPECT_EQ( n / 2 + n, pool.size() );
	EXPECT_EQ( static_cast< int >( pool.size() ), Counted::alive );
}

/// 消した場所は次に置くときに使い回す（場所の数が増え続けない）
TEST( StablePoolTest, FreedSlotsAreReused )
{
	StablePool< Counted > pool;

	for ( EntityId id = 0; id < 100; id++ )
	{
		pool.emplace( id );
	}

	const auto slots = pool.debug_slot_count();

	// 作っては消すのを繰り返す（弾・粒子のような使い方）
	for ( EntityId id = 100; id < 1100; id++ )
	{
		pool.erase( id - 100 );
		pool.emplace( id );
	}

	EXPECT_EQ( slots, pool.debug_slot_count() );
	EXPECT_EQ( 100u, pool.size() );
}

/// for_each は置いている値だけを、場所の順に回す（消した場所は飛ばす）
TEST( StablePoolTest, ForEachVisitsLiveValuesInSlotOrder )
{
	StablePool< Counted > pool;

	for ( EntityId id = 0; id < 5; id++ )
	{
		pool.emplace( id * 10, static_cast< int >( id ) );
	}

	pool.erase( 10 );
	pool.erase( 30 );

	std::vector< EntityId > ids;
	std::vector< int > values;

	pool.for_each( [ & ] ( EntityId id, Counted& c ) { ids.push_back( id ); values.push_back( c.value ); } );

	EXPECT_EQ( ( std::vector< EntityId >{ 0, 20, 40 } ), ids );
	EXPECT_EQ( ( std::vector< int >{ 0, 2, 4 } ), values );
}

/// 置き場を壊すと、残っている値も壊す
TEST( StablePoolTest, DestructorDestroysRemainingValues )
{
	{
		StablePool< Counted > pool;
		pool.emplace( 1 );
		pool.emplace( 2 );
		pool.emplace( 3 );
		pool.erase( 2 );
		EXPECT_EQ( 2, Counted::alive );
	}

	EXPECT_EQ( 0, Counted::alive );
}

/// ムーブだけできる値・1 ページに 1 つしか入らない大きな値も置ける
TEST( StablePoolTest, MoveOnlyAndHugeTypes )
{
	StablePool< MoveOnly > move_only;
	auto* m = move_only.emplace( 7 );
	ASSERT_NE( nullptr, m );
	EXPECT_EQ( 5, * m->p );

	static_assert( StablePool< Huge >::PAGE_CAPACITY == 1 );

	StablePool< Huge > huge;
	auto* h0 = huge.emplace( 0 );
	auto* h1 = huge.emplace( 1 );
	h0->value = 10;
	h1->value = 11;
	huge.erase( 0 );
	auto* h2 = huge.emplace( 2 );

	EXPECT_EQ( h0, h2 );		// 空いた場所を使い回す
	EXPECT_EQ( 11, huge.find( 1 )->value );
	EXPECT_EQ( 0, huge.find( 2 )->value );
	EXPECT_EQ( 2u, huge.debug_slot_count() );
}
