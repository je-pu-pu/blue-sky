#include "pch.h"

#include <blue_sky/ActiveObjectManager.h>

using blue_sky::ActiveObjectManager;

TEST( TargetLocationTest, 残りが_0_1_未満なら着いたことにする )
{
	EXPECT_TRUE( ActiveObjectManager::is_target_location_reached( 0.05f, 1.f, 1.f / 60.f ) );
	EXPECT_FALSE( ActiveObjectManager::is_target_location_reached( 0.5f, 1.f, 1.f / 60.f ) );
}

TEST( TargetLocationTest, このフレームで進める距離以内なら着いたことにする )
{
	// 速さ 30 で 1/30 秒のフレームなら 1 進む。以前は 0.1 未満に入るまで目標の周りを行ったり来たりしていた
	EXPECT_TRUE( ActiveObjectManager::is_target_location_reached( 0.8f, 30.f, 1.f / 30.f ) );
	EXPECT_FALSE( ActiveObjectManager::is_target_location_reached( 1.2f, 30.f, 1.f / 30.f ) );
}

TEST( TargetLocationTest, 経過時間が_0_でも_0_1_の判定は効く )
{
	EXPECT_TRUE( ActiveObjectManager::is_target_location_reached( 0.09f, 10.f, 0.f ) );
	EXPECT_FALSE( ActiveObjectManager::is_target_location_reached( 0.2f, 10.f, 0.f ) );
}
