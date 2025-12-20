#pragma once

#include <core/sound/SoundFilter.h>
#include <cmath>
#include <numbers>
#include <ranges>

namespace core::sound::filter
{

class BiquadFilter : public SoundFilter
{
private:
    /**
	 * チャンネルごとの状態を管理する構造体
     */
    struct ChannelState
    {
        float x1 = 0.0f; // 前の入力サンプル
        float x2 = 0.0f; // 2つ前の入力サンプル
        float y1 = 0.0f; // 前の出力サンプル
		float y2 = 0.0f; // 2つ前の出力サンプル

        void reset()
        {
            x1 = x2 = 0.0f;
            y1 = y2 = 0.0f;
		}
    };

private:
    std::vector< ChannelState > channel_state_list_;

public:
    enum class FilterType
    {
        Lowpass,
        Highpass,
        Bandpass,
        Notch
    };

    BiquadFilter( int channels, float sampleRate, float cutoffFreq, float Q, FilterType type = FilterType::Lowpass )
        : sampleRate( sampleRate )
        , cutoffFreq( cutoffFreq )
        , Q( Q )
        , filterType( type )
    {
		channel_state_list_.resize( channels );

        calculateCoefficients();
    }

    ~BiquadFilter() override
    {

    }

    void reset()
    {
        std::ranges::for_each( channel_state_list_, [] ( auto state ) { state.reset(); } );
    }

    void setCutoff( float newCutoffFreq )
    {
        cutoffFreq = newCutoffFreq;
        calculateCoefficients();
    }

    void setQ( float newQ )
    {
        Q = newQ;
        calculateCoefficients();
    }

    void process( float* data, size_t frame_count ) override
    {
        for ( size_t n = 0; n < frame_count; ++n )
        {
            for ( size_t ch = 0; ch < channel_state_list_.size(); ++ch )
            {
                data[ n * channel_state_list_.size() + ch ] = process( channel_state_list_[ ch ], data[ n * channel_state_list_.size() + ch ] );
            }
		}

    }

    float process( ChannelState& channel_state, float x0 ) const
    {
        float y0 = b0 * x0 + b1 * channel_state.x1 + b2 * channel_state.x2 - a1 * channel_state.y1 - a2 * channel_state.y2;

        channel_state.x2 = channel_state.x1;
        channel_state.x1 = x0;
        channel_state.y2 = channel_state.y1;
        channel_state.y1 = y0;

        return y0;
    }

private:
    float sampleRate;
    float cutoffFreq;
    float Q;
    FilterType filterType;

    float a1, a2, b0, b1, b2;

    void calculateCoefficients()
    {
        float omega = 2.0f * std::numbers::pi_v< float > * cutoffFreq / sampleRate;

        float alpha = std::sin( omega ) / (2.0f * Q);
        float cos_omega = std::cos( omega );

        // フィルタタイプに応じて係数計算
        switch ( filterType )
        {
            case FilterType::Lowpass:
                b0 = (1.0f - cos_omega) / 2.0f;
                b1 = 1.0f - cos_omega;
                b2 = (1.0f - cos_omega) / 2.0f;
                break;

            case FilterType::Highpass:
                b0 = (1.0f + cos_omega) / 2.0f;
                b1 = -(1.0f + cos_omega);
                b2 = (1.0f + cos_omega) / 2.0f;
                break;

            case FilterType::Bandpass:
                b0 = sinf(omega) / 2.0f;
                b1 = 0.0f;
                b2 = -sinf(omega) / 2.0f;
                break;

            case FilterType::Notch:
                b0 = 1.0f;
                b1 = -2.0f * cos_omega;
                b2 = 1.0f;
                break;
        }

        float a0 = 1.0f + alpha;
        a1 = -2.0f * cos_omega;
        a2 = 1.0f - alpha;

        b0 /= a0;
        b1 /= a0;
        b2 /= a0;
        a1 /= a0;
        a2 /= a0;
    }
};

} // namespace core::sound::filter