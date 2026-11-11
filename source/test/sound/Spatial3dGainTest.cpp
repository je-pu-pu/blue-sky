#include "pch.h"

#include <core/sound/PortAudio/SoundEngine.h>

// ---- 3D の音（PortAudio の自前の方向＋距離）: 音源の位置から左右のゲインを求める ----
// 耳で聞かなくても確かめられるよう、ゲインの計算だけを取り出して確かめる

namespace
{

using core::Vector3;
using core::sound::port_audio::SoundEngine;

struct Gains
{
	float l = 0.f;
	float r = 0.f;
};

/// リスナーは原点で +Z を向き、上は +Y。減衰しない距離 2m・鳴らない距離 45m（エンジンの既定と同じ）
Gains gains_at( const Vector3& source )
{
	Gains g;
	SoundEngine::compute_3d_gains( source, Vector3( 0.f, 0.f, 0.f ), Vector3( 0.f, 0.f, 1.f ), Vector3( 0.f, 1.f, 0.f ), 2.f, 45.f, g.l, g.r );
	return g;
}

} // namespace

TEST( Spatial3dGainTest, 正面の音は左右が同じ大きさ )
{
	const auto g = gains_at( Vector3( 0.f, 0.f, 1.f ) );
	EXPECT_NEAR( g.l, g.r, 1e-5f );
	EXPECT_NEAR( g.l, 0.70710678f, 1e-4f );		// 等パワー（中央は 1/√2）
}

TEST( Spatial3dGainTest, 右の音は右が大きく左が鳴らない )
{
	// このエンジンの座標では、+Z を向いたときの右は +X（左右が逆にならないこと）
	const auto g = gains_at( Vector3( 1.f, 0.f, 0.f ) );
	EXPECT_NEAR( g.r, 1.f, 1e-5f );
	EXPECT_NEAR( g.l, 0.f, 1e-5f );
}

TEST( Spatial3dGainTest, 左の音は左が大きく右が鳴らない )
{
	const auto g = gains_at( Vector3( -1.f, 0.f, 0.f ) );
	EXPECT_NEAR( g.l, 1.f, 1e-5f );
	EXPECT_NEAR( g.r, 0.f, 1e-5f );
}

TEST( Spatial3dGainTest, 遠いほど小さく_鳴らない距離より遠いと無音 )
{
	const auto near_ = gains_at( Vector3( 0.f, 0.f, 2.f ) );		// 減衰しない距離
	const auto mid   = gains_at( Vector3( 0.f, 0.f, 23.5f ) );		// ちょうど真ん中
	const auto far_  = gains_at( Vector3( 0.f, 0.f, 50.f ) );

	EXPECT_NEAR( near_.l, 0.70710678f, 1e-4f );
	EXPECT_NEAR( mid.l, 0.70710678f * 0.5f, 1e-4f );
	EXPECT_EQ( far_.l, 0.f );
	EXPECT_EQ( far_.r, 0.f );
}
