#include "pch.h"

// フレーム毎コヒーレントノイズ（空間白色＋時間 EMA 平滑）の単体テスト。
// EMA で分散が減るぶんを固定係数で戻し、出力の空間 std が ≈1 になることを検証する。
#include <blue_sky/graphics/CoherentNoise.h>

#include <vector>
#include <random>
#include <cmath>
#include <iostream>

namespace {

// 空間サンプルの平均・標準偏差
std::pair< double, double > mean_std( const std::vector< float >& v )
{
	double sum = 0.0, sq = 0.0;
	for ( float x : v ) { sum += x; sq += static_cast< double >( x ) * x; }
	const double n = static_cast< double >( v.size() );
	const double mean = sum / n;
	const double var = sq / n - mean * mean;
	return { mean, std::sqrt( var > 0.0 ? var : 0.0 ) };
}

} // namespace

TEST( CoherentNoiseTest, StdApproxOneAtSteadyState )
{
	// 2 の冪の N(0,1) テーブル
	constexpr size_t TBL = 1u << 16;
	std::vector< float > table( TBL );
	std::mt19937 rng( 42u );
	std::normal_distribution< float > g( 0.f, 1.f );
	for ( auto& x : table ) x = g( rng );

	constexpr size_t n = 4096;
	std::vector< float > prev, out( n );

	for ( float a : { 0.2f, 0.35f, 0.6f, 1.0f } )
	{
		// EMA が定常に達するまで回す（毎フレーム別オフセット）
		for ( int frame = 0; frame < 100; ++frame )
		{
			const size_t off = static_cast< size_t >( rng() ) & ( TBL - 1 );
			blue_sky::graphics::step_coherent_noise( table, prev, out, off, a );
		}
		const auto [ mean, sd ] = mean_std( out );
		std::cout << "[CoherentNoise] a=" << a << " mean=" << mean << " std=" << sd << std::endl;
		EXPECT_NEAR( sd, 1.0, 0.15 ) << "a=" << a;   // 固定スケールで std≈1 に戻る
		EXPECT_NEAR( mean, 0.0, 0.15 ) << "a=" << a; // 白色ゆえ平均≈0
	}
}

TEST( CoherentNoiseTest, EmptyTableIsSafe )
{
	std::vector< float > empty, prev, out( 16, 123.f );
	blue_sky::graphics::step_coherent_noise( empty, prev, out, 0, 0.5f );
	SUCCEED();  // クラッシュしなければよい（table 空は何もしない）
}
