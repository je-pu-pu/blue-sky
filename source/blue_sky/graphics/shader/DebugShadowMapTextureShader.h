#pragma once

#include <blue_sky/graphics/shader/BaseShader.h>

namespace blue_sky::graphics::shader
{

/**
 * シャドウマップテクスチャデバッグ表示用シェーダー
 *
 */
class DebugShadowMapTextureShader : public BaseShader
{
private:
	TextureSlot texture_;	///< 管理しているテクスチャはハンドルで持ち、描くたびに引く（TextureSlot）
	RenderSetting render_setting_;

protected:
	Texture* get_texture() const { return resolve_texture( texture_ ); }
	Texture* get_texture_at( uint_t n ) override { return n == 0 ? resolve_texture( texture_ ) : nullptr; }
	const Texture* get_texture_at( uint_t n ) const override { return n == 0 ? resolve_texture( texture_ ) : nullptr; }
	void set_texture_at( uint_t, Texture* t ) override { set_texture( t ); }

public:
	DebugShadowMapTextureShader( GraphicsManager* graphics_manager )
		: BaseShader( graphics_manager )
		, render_setting_( graphics_manager, "2d", "debug_shadow_map_texture" )
	{

	}

	void reload() override
	{
		render_setting_.reload();
	}

	DebugShadowMapTextureShader* clone() const override { return new DebugShadowMapTextureShader( *this ); }

	void set_texture( Texture* t ) override { texture_.set( * get_graphics_manager(), t ); }

	void bind() const override
	{
		get_object_constant_buffer()->bind_to_ps();
		bind_texture_to_ps( texture_, 0 );
	}

	void render( const Mesh* mesh, uint_t n ) const override
	{
		if ( is_texture_lost( texture_ ) )
		{
			return;
		}

		get_graphics_manager()->set_input_layout( render_setting_.get_input_layout() );
		get_graphics_manager()->set_primitive_topology( PrimitiveTopology::TRIANGLE_LIST );
		get_graphics_manager()->render_technique( render_setting_.get_effect_technique(), [=] { bind(); mesh->render( n ); } );
	}
};

} // namespace blue_sky::graphics::shader
