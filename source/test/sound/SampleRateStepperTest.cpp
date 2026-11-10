#include "pch.h"

#include <core/sound/SampleRateStepper.h>

using core::sound::SampleRateStepper;

namespace
{

/// 出力 frames フレームの間に、音のフレームがいくつ進むか
size_t advance( SampleRateStepper& stepper, int frames )
{
	size_t total = 0;

	for ( int n = 0; n < frames; n++ )
	{
		total += stepper.step();
	}

	return total;
}

} // namespace

TEST( SampleRateStepperTest, 同じ周波数なら毎フレーム_1_つ進む )
{
	SampleRateStepper stepper( 44100, 44100 );

	for ( int n = 0; n < 100; n++ )
	{
		EXPECT_EQ( 1u, stepper.step() );
	}
}

TEST( SampleRateStepperTest, 半分の周波数の音は半分の速さで進む )
{
	// 以前は 22050 Hz の音楽を 44100 Hz の出力でそのまま読み、倍の速さで鳴っていた
	SampleRateStepper stepper( 22050, 44100 );

	EXPECT_EQ( 22050u, advance( stepper, 44100 ) );
}

TEST( SampleRateStepperTest, 割り切れない比でも長い間にずれない )
{
	SampleRateStepper stepper( 48000, 44100 );

	EXPECT_EQ( 48000u, advance( stepper, 44100 ) );
}

TEST( SampleRateStepperTest, reset_で端数を捨てる )
{
	SampleRateStepper stepper( 22050, 44100 );

	EXPECT_EQ( 0u, stepper.step() );	// 0.5
	stepper.reset();
	EXPECT_EQ( 0u, stepper.step() );	// 0.5 から数え直す
	EXPECT_EQ( 1u, stepper.step() );
}

TEST( SampleRateStepperTest, 周波数が不正なら同じ周波数として扱う )
{
	SampleRateStepper stepper( 0, 44100 );

	EXPECT_EQ( 1u, stepper.step() );
}
