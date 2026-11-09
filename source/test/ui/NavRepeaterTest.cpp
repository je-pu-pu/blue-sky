#include "pch.h"

#include <core/ui/NavRepeater.h>

using core::ui::NavRepeater;

// 浮動小数の誤差に左右されないよう、時間は 2 進数で正確に表せる値（0.25, 0.125 など）を使う

TEST( NavRepeaterTest, 押した瞬間に1回だけ発火する )
{
	NavRepeater r( 0.5f, 0.125f );

	EXPECT_TRUE( r.update( true, 0.0625f ) );
	EXPECT_FALSE( r.update( true, 0.0625f ) );
	EXPECT_FALSE( r.update( true, 0.0625f ) );
}

TEST( NavRepeaterTest, 初回遅延の後は一定間隔で発火する )
{
	NavRepeater r( 0.5f, 0.125f );
	r.update( true, 0.f );

	EXPECT_FALSE( r.update( true, 0.25f ) );   // 0.25
	EXPECT_TRUE( r.update( true, 0.25f ) );    // 0.5 初回遅延に到達
	EXPECT_FALSE( r.update( true, 0.0625f ) ); // 0.5625
	EXPECT_TRUE( r.update( true, 0.0625f ) );  // 0.625
	EXPECT_TRUE( r.update( true, 0.125f ) );   // 0.75
}

TEST( NavRepeaterTest, 長いフレームでも1回しか発火せず次回は今から間隔後 )
{
	NavRepeater r( 0.5f, 0.125f );
	r.update( true, 0.f );

	EXPECT_TRUE( r.update( true, 1.0f ) );     // 1.0 秒分たまっても 1 回
	EXPECT_FALSE( r.update( true, 0.0625f ) ); // 1.0625（次は 1.125）
	EXPECT_TRUE( r.update( true, 0.0625f ) );  // 1.125
}

TEST( NavRepeaterTest, 離すとリセットされ次の押下で即発火する )
{
	NavRepeater r( 0.5f, 0.125f );
	r.update( true, 0.f );
	r.update( true, 0.75f );

	EXPECT_FALSE( r.update( false, 0.0625f ) );
	EXPECT_TRUE( r.update( true, 0.0625f ) );
	EXPECT_FALSE( r.update( true, 0.125f ) );
}

TEST( NavRepeaterTest, スティックの傾きが小さいほど繰り返しがゆっくりになる )
{
	auto count_fires = [] ( float_t strength )
	{
		core::ui::NavRepeater r( 0.35f, 0.08f );
		int fires = 0;
		for ( int i = 0; i < 120; i++ ) { if ( r.update( true, 1.f / 60.f, strength ) ) fires++; }		// 2 秒押し続ける
		return fires;
	};

	const int full = count_fires( 1.f );
	const int half = count_fires( 0.5f );
	EXPECT_GT( full, half );
	EXPECT_GT( half, 1 );
}
