#include "pch.h"

#include <core/AsyncLoader.h>

#include <atomic>
#include <string>
#include <thread>

TEST( AsyncLoaderTest, 仕事は作業スレッドで行い結果の処理はメインスレッドで呼ぶ )
{
	core::AsyncLoader loader;

	const std::thread::id main_thread = std::this_thread::get_id();
	std::thread::id work_thread;
	std::thread::id done_thread;
	int result = 0;

	loader.enqueue< int >( [&] { work_thread = std::this_thread::get_id(); return 42; }, [&] ( int r ) { done_thread = std::this_thread::get_id(); result = r; } );
	loader.wait_all();

	EXPECT_EQ( 42, result );
	EXPECT_NE( main_thread, work_thread );		// 仕事は別のスレッド
	EXPECT_EQ( main_thread, done_thread );		// 結果の処理はメインスレッド
	EXPECT_EQ( 0u, loader.get_pending_count() );
}

TEST( AsyncLoaderTest, 積んだ順に処理し例外を投げた仕事があっても止まらない )
{
	core::AsyncLoader loader;
	std::string order;

	loader.enqueue< char >( [] { return 'a'; }, [&] ( char c ) { order += c; } );
	loader.enqueue< char >( [] () -> char { throw std::runtime_error( "失敗" ); }, [&] ( char c ) { order += c; } );
	loader.enqueue< char >( [] { return 'c'; }, [&] ( char c ) { order += c; } );
	loader.wait_all();

	EXPECT_EQ( "ac", order );		// 例外の仕事は結果の処理を呼ばない。後の仕事は続く
}

TEST( AsyncLoaderTest, 結果の処理はdispatch_completedを呼ぶまで呼ばれない )
{
	core::AsyncLoader loader;
	std::atomic< bool > worked = false;
	bool done = false;

	loader.enqueue< int >( [&] { worked = true; return 1; }, [&] ( int ) { done = true; } );

	while ( loader.get_ready_count() == 0 ) { std::this_thread::yield(); }		// 作業スレッドが終わるのを待つ
	EXPECT_TRUE( worked );

	EXPECT_FALSE( done );						// まだメインスレッドで処理していない
	EXPECT_EQ( 1u, loader.dispatch_completed() );
	EXPECT_TRUE( done );
}

TEST( AsyncLoaderTest, 破棄すると残りの仕事を捨てて止まる )
{
	std::atomic< int > count = 0;

	{
		core::AsyncLoader loader;

		for ( int i = 0; i < 100; i++ )
		{
			loader.enqueue< int >( [&] { std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) ); return ++count; }, [] ( int ) { } );
		}
	}	// ここで止まる（100 件すべては待たない）

	EXPECT_LT( count.load(), 100 );
}
