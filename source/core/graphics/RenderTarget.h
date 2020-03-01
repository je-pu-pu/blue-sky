#pragma once

#include <type/type.h>

namespace core::graphics
{

/**
 * レンダリング結果を書き込む事ができるオブジェクトのインターフェイス
 *
 */
class RenderTarget
{
public:
    RenderTarget() { }
    virtual ~RenderTarget() { }

    virtual void activate() = 0;

}; // RenderTarget

} // namespace core::graphics
