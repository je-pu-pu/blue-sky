#pragma once

#include "Texture.h"
#include "RenderTarget.h"

namespace core::graphics
{

/**
 * レンダリング結果を書き込むことができ、かつ、シェーダーリソースとして使用できるテクスチャ 
 *
 */
class RenderTargetTexture : public Texture, public RenderTarget
{
public:
	RenderTargetTexture() { }
	virtual ~RenderTargetTexture() { }

}; // RenderTargetTexture

} // namespace core::graphics
