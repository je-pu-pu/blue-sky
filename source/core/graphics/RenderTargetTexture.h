#pragma once

#include "RenderTarget.h"
#include "ShaderResource.h"

namespace core::graphics
{

/**
 * レンダリング結果を書き込むことができ、かつ、シェーダーリソースとして使用できるテクスチャ 
 *
 */
class RenderTargetTexture : public RenderTarget, public ShaderResource
{
public:
	RenderTargetTexture() { }
	virtual ~RenderTargetTexture() { }

}; // RenderTargetTexture

} // namespace core::graphics
