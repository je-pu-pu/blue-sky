#pragma once

#include <core/graphics/Shader.h>
#include <core/graphics/PixelFormat.h>
#include <core/ResourceManager.h>

namespace core
{
	class SkinningAnimationSet;
}

namespace core::graphics
{

class Shader;
class Texture;
class RenderTarget;
class RenderTargetTexture;
class BackBufferTexture;
class ConstantBuffer;

/**
 * グラフィック管理クラス
 *
 * @todo 太らせる
 */
class GraphicsManager
{
public:
	using Shader						= core::graphics::Shader;

	using Texture						= core::graphics::Texture;
	using RenderTarget					= core::graphics::RenderTarget;
	using RenderTargetTexture			= core::graphics::RenderTargetTexture;
	using BackBufferTexture				= core::graphics::BackBufferTexture;

	using ConstantBuffer				= core::graphics::ConstantBuffer;

	using SkinningAnimationSet			= core::SkinningAnimationSet;

	using ShaderManager					= core::ResourceManager< Shader >;
	using TextureManager				= core::ResourceManager< Texture >;
	using SkinningAnimationSetManager	= core::ResourceManager< SkinningAnimationSet >;

private:
	ShaderManager				shader_manager_;

protected:
	const ShaderManager& get_shader_manager() const { return shader_manager_; }

public:
	GraphicsManager() { }
	virtual ~GraphicsManager() { }

	// Shader
	template< typename Type, typename ... Args > Type* create_shader( Args ... args ) { return shader_manager_.create< Type >( args ... ); }
	template< typename Type, typename ... Args > Type* create_named_shader( const char_t* name, Args ... args ) { return shader_manager_.create_named< Type >( name, args ... ); }
	template< typename Type=Shader > Type* get_shader( const char_t* name ) { return shader_manager_.get< Type >( name ); }
	Shader* clone_shader( const Shader* s ) { Shader* s2 = s->clone(); shader_manager_.add( s2 ); return s2; }

	const ShaderManager::ResourceList& get_shader_list() const { return shader_manager_.get_resource_list(); }

	virtual RenderTargetTexture* create_render_target_texture() = 0;
	virtual RenderTargetTexture* create_render_target_texture( PixelFormat ) = 0;

	virtual BackBufferTexture* get_back_buffer_texture() = 0;
	virtual void set_render_target( RenderTarget* );
	virtual void set_default_render_target() = 0;

	virtual void setup_rendering() = 0;
	virtual void render_background() const = 0;

	virtual void set_current_object_constant_buffer( const ConstantBuffer* ) const = 0;
	virtual void set_current_skinning_constant_buffer( const ConstantBuffer* ) const = 0;

	virtual void set_post_effect_shader( Shader* ) = 0;
	virtual void render_post_effect( Texture* ) = 0;
	virtual void render_post_effect( Texture*, RenderTarget* ) = 0;

	virtual void render_fader() const = 0;

	virtual void clear_debug_bullet() const = 0;
	virtual void render_debug_bullet() const = 0;

}; // class GraphicsManager

} // namespace core::graphics
