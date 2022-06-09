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
	using ConstantBuffer				= core::graphics::ConstantBuffer;

public:
	GraphicsManager() { }
	virtual ~GraphicsManager() { }

	virtual RenderTargetTexture* create_render_target_texture() = 0;
	virtual RenderTargetTexture* create_render_target_texture( PixelFormat ) = 0;

	virtual void setup_rendering() = 0;
	virtual void render_background() const = 0;

	virtual void set_current_object_constant_buffer( const ConstantBuffer* ) const = 0;
	virtual void set_current_skinning_constant_buffer( const ConstantBuffer* ) const = 0;

	virtual void render_fader() const = 0;

	virtual void clear_debug_bullet() const = 0;
	virtual void render_debug_bullet() const = 0;

}; // class GraphicsManager

} // namespace core::graphics
