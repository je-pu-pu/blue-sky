#pragma once

#include <cstddef>

namespace core::sound
{

/**
 * 周波数の違う音を、出力の周波数で読み進めるための位置
 *
 * 出力の 1 フレームごとに step() を呼び、返った数だけ音のフレームを進める
 * （22050 Hz の音を 44100 Hz で鳴らすなら 2 回に 1 回 1 を返す。同じ周波数なら毎回 1）
 */
class SampleRateStepper
{
private:
	double step_;				///< 出力 1 フレームあたりに進む音のフレーム数
	double fraction_ = 0.0;		///< まだ進んでいない端数

public:
	SampleRateStepper( int source_rate, int output_rate )
		: step_( ( source_rate > 0 && output_rate > 0 ) ? static_cast< double >( source_rate ) / static_cast< double >( output_rate ) : 1.0 )
	{ }

	/// 出力 1 フレーム分進め、音のフレームをいくつ進めるかを返す
	std::size_t step()
	{
		fraction_ += step_;

		const auto frames = static_cast< std::size_t >( fraction_ );
		fraction_ -= static_cast< double >( frames );

		return frames;
	}

	/// 端数を捨てる（頭から鳴らし直すとき）
	void reset() { fraction_ = 0.0; }

}; // class SampleRateStepper

} // namespace core::sound
