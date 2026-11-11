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

	/**
	 * シェーダーで普通のテクスチャ（Texture2D の Sample）として読める形にして返す
	 *
	 * MSAA の描画先は、そのままでは Texture2D として読めない（読むと真っ黒になる）。MSAA なら MSAA なしのテクスチャへ解決して、それを返す。
	 * MSAA でなければ自分を返す。ポストエフェクトのように描画結果を読むときに呼ぶ
	 */
	virtual Texture* resolve_for_sampling() { return this; }

}; // RenderTargetTexture

} // namespace core::graphics
