#include "pch.h"

#include <core/graphics/Pixel2DSpace.h>

using core::graphics::Pixel2DSpace;

TEST( Pixel2DSpaceTest, 画面の四隅が描画用の座標の四隅に写る )
{
	const Pixel2DSpace s{ 1920.f, 1080.f, 0.f };
	const float_t aspect = 1920.f / 1080.f;

	ASSERT_TRUE( s.is_valid() );

	// 左上 (0, 0) → (-aspect, +1)、右下 (w, h) → (+aspect, -1)
	EXPECT_FLOAT_EQ( -aspect, s.to_view_x( 0.f ) );
	EXPECT_FLOAT_EQ( 1.f, s.to_view_y( 0.f ) );
	EXPECT_FLOAT_EQ( aspect, s.to_view_x( 1920.f ) );
	EXPECT_FLOAT_EQ( -1.f, s.to_view_y( 1080.f ) );

	// 中央は原点
	EXPECT_NEAR( 0.f, s.to_view_x( 960.f ), 1e-6f );
	EXPECT_NEAR( 0.f, s.to_view_y( 540.f ), 1e-6f );

	// 正射影は描画用の座標の範囲そのもの（幅 2 × aspect、高さ 2）
	EXPECT_FLOAT_EQ( 2.f * aspect, s.ortho_width() );
	EXPECT_FLOAT_EQ( 2.f, Pixel2DSpace::ortho_height() );
}

TEST( Pixel2DSpaceTest, 大きさが0なら描画できない )
{
	EXPECT_FALSE( ( Pixel2DSpace{ 0.f, 0.f, 0.f } ).is_valid() );
	EXPECT_FALSE( ( Pixel2DSpace{ 1280.f, 0.f, 0.f } ).is_valid() );
}
