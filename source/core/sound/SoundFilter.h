#pragma once

#include <common/math.h>

namespace core::sound
{

/**
 * サウンドフィルタ基底クラス
 *
 */
class SoundFilter
{
private:
	float mix_ = 1.f;

protected:
	float mix( float dry, float wet ) const
	{
		return dry * ( 1.f - mix_ ) + wet * mix_;
	}

public:
    virtual ~SoundFilter() = default;
    virtual void process( float* buffer, size_t frames ) = 0;

	void set_mix( float mix ) { mix_ = ::math::clamp( mix, 0.f, 1.f ); }
};

} // namespace core::sound
