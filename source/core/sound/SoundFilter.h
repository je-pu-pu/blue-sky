#pragma once

#include <cstddef>

namespace core::sound
{

/**
 * サウンドフィルタ基底クラス
 *
 */
class SoundFilter
{
public:
    virtual ~SoundFilter() = default;
    virtual void process( float* buffer, size_t frames ) = 0;
};

} // namespace core::sound
