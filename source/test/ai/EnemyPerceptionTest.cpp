#include "pch.h"

// 敵AIの知覚・警戒判定（純関数）の単体テスト。
// 視界錐の内外判定・awareness の増減・警戒状態遷移（ヒステリシス）を検証する。
#include <blue_sky/ai/EnemyPerception.h>

using core::math::Vector;
using blue_sky::ai::AlertState;
using blue_sky::ai::ViewCone;

namespace {

// 既定の錐: 距離20m / 水平半画角60deg / 上方許容20deg
ViewCone default_cone()
{
	ViewCone c;
	c.view_distance      = 20.f;
	c.horizontal_half_fov = 1.0472f; // 60deg
	c.vertical_up_limit  = 0.3491f;  // 20deg
	return c;
}

} // namespace

// --- 視界錐 -------------------------------------------------------------

TEST( EnemyPerceptionTest, TargetStraightAheadIsVisible )
{
	const Vector eye( 0.f, 1.f, 0.f );
	const Vector forward( 0.f, 0.f, 1.f ); // +Z 前方
	const Vector target( 0.f, 1.f, 5.f );  // 正面 5m
	EXPECT_TRUE( blue_sky::ai::is_in_view_cone( eye, forward, default_cone(), target ) );
}

TEST( EnemyPerceptionTest, TargetBehindIsNotVisible )
{
	const Vector eye( 0.f, 1.f, 0.f );
	const Vector forward( 0.f, 0.f, 1.f );
	const Vector target( 0.f, 1.f, -5.f ); // 真後ろ
	EXPECT_FALSE( blue_sky::ai::is_in_view_cone( eye, forward, default_cone(), target ) );
}

TEST( EnemyPerceptionTest, TargetBeyondDistanceIsNotVisible )
{
	const Vector eye( 0.f, 1.f, 0.f );
	const Vector forward( 0.f, 0.f, 1.f );
	const Vector target( 0.f, 1.f, 25.f ); // 25m > 20m
	EXPECT_FALSE( blue_sky::ai::is_in_view_cone( eye, forward, default_cone(), target ) );
}

TEST( EnemyPerceptionTest, TargetInsideHorizontalFovIsVisible )
{
	const Vector eye( 0.f, 1.f, 0.f );
	const Vector forward( 0.f, 0.f, 1.f );
	// 正面から水平 45deg（半画角 60deg 以内）: x=5, z=5 で 45deg
	const Vector target( 5.f, 1.f, 5.f );
	EXPECT_TRUE( blue_sky::ai::is_in_view_cone( eye, forward, default_cone(), target ) );
}

TEST( EnemyPerceptionTest, TargetOutsideHorizontalFovIsNotVisible )
{
	const Vector eye( 0.f, 1.f, 0.f );
	const Vector forward( 0.f, 0.f, 1.f );
	// 正面から水平 75deg（半画角 60deg 超）: x=5, z=1.34 で ≈75deg
	const Vector target( 5.f, 1.f, 1.34f );
	EXPECT_FALSE( blue_sky::ai::is_in_view_cone( eye, forward, default_cone(), target ) );
}

// 「上を見ない」＝高いほど安全 のテーマをコードで担保する中心的なテスト。
TEST( EnemyPerceptionTest, TargetHighAboveIsNotVisible )
{
	const Vector eye( 0.f, 1.f, 0.f );
	const Vector forward( 0.f, 0.f, 1.f );
	// 正面 5m だが 10m 上空（仰角 ≈63deg > 20deg 制限）
	const Vector target( 0.f, 11.f, 5.f );
	EXPECT_FALSE( blue_sky::ai::is_in_view_cone( eye, forward, default_cone(), target ) );
}

TEST( EnemyPerceptionTest, TargetSlightlyAboveWithinLimitIsVisible )
{
	const Vector eye( 0.f, 1.f, 0.f );
	const Vector forward( 0.f, 0.f, 1.f );
	// 正面 10m で 1m 上（仰角 ≈5.7deg < 20deg 制限）
	const Vector target( 0.f, 2.f, 10.f );
	EXPECT_TRUE( blue_sky::ai::is_in_view_cone( eye, forward, default_cone(), target ) );
}

// --- awareness --------------------------------------------------------

TEST( EnemyPerceptionTest, AwarenessRisesWhenVisibleAndClampsToOne )
{
	float a = 0.f;
	for ( int i = 0; i < 100; ++i )
	{
		a = blue_sky::ai::step_awareness( a, /*visible=*/true, 0.1f, /*gain=*/2.f, /*decay=*/1.f );
	}
	EXPECT_FLOAT_EQ( a, 1.f ); // 上限で飽和
}

TEST( EnemyPerceptionTest, AwarenessDecaysWhenNotVisibleAndClampsToZero )
{
	float a = 1.f;
	for ( int i = 0; i < 100; ++i )
	{
		a = blue_sky::ai::step_awareness( a, /*visible=*/false, 0.1f, 2.f, 1.f );
	}
	EXPECT_FLOAT_EQ( a, 0.f ); // 下限で飽和
}

// --- 状態遷移（ヒステリシス） -----------------------------------------

TEST( EnemyPerceptionTest, StateEscalatesAsAwarenessRises )
{
	using blue_sky::ai::next_alert_state;
	AlertState s = AlertState::Unaware;
	s = next_alert_state( s, 0.0f );  EXPECT_EQ( s, AlertState::Unaware );
	s = next_alert_state( s, 0.30f ); EXPECT_EQ( s, AlertState::Suspicious );
	s = next_alert_state( s, 0.65f ); EXPECT_EQ( s, AlertState::Searching );
	s = next_alert_state( s, 1.0f );  EXPECT_EQ( s, AlertState::Alarmed );
}

TEST( EnemyPerceptionTest, StateDeescalatesOneStepWithHysteresis )
{
	using blue_sky::ai::next_alert_state;
	// 警報からの下り。alarmed_down=0.70 未満で捜索へ、いきなり無警戒には戻らない。
	AlertState s = AlertState::Alarmed;
	s = next_alert_state( s, 0.80f ); EXPECT_EQ( s, AlertState::Alarmed );   // まだ保持
	s = next_alert_state( s, 0.50f ); EXPECT_EQ( s, AlertState::Searching ); // 一段下がる
	s = next_alert_state( s, 0.20f ); EXPECT_EQ( s, AlertState::Suspicious );// さらに一段
	s = next_alert_state( s, 0.05f ); EXPECT_EQ( s, AlertState::Unaware );   // 完全に戻る
}

TEST( EnemyPerceptionTest, SuspicionHoldsInHysteresisBand )
{
	using blue_sky::ai::next_alert_state;
	// 不審に入った後、下げ閾値(0.10)と上げ閾値(0.60)の間では不審を維持する。
	AlertState s = AlertState::Suspicious;
	s = next_alert_state( s, 0.20f ); EXPECT_EQ( s, AlertState::Suspicious );
	s = next_alert_state( s, 0.15f ); EXPECT_EQ( s, AlertState::Suspicious );
}
