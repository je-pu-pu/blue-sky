#pragma once

#include <core/graphics/PixelFormat.h>

namespace core::graphics
{

class RenderTargetTexture;

/**
 * グラフィック管理クラス
 *
 * @todo 太らせる
 */
class GraphicsManager
{
public:
	using RenderTargetTexture			= core::graphics::RenderTargetTexture;

public:
	GraphicsManager() { }
	virtual ~GraphicsManager() { }


	virtual RenderTargetTexture* create_render_target_texture() = 0;
	virtual RenderTargetTexture* create_render_target_texture( PixelFormat ) = 0;

}; // class GraphicsManager

} // namespace core::graphics
