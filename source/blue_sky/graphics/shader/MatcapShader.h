#pragma once

#include <blue_sky/graphics/shader/BaseShader.h>

namespace blue_sky::graphics::shader
{

/**
 * Matcap シェーダー
 *
 */
class MatcapShader : public BaseShader
{
private:
	TextureSlot texture_;	///< 管理しているテクスチャはハンドルで持ち、描くたびに引く（TextureSlot）
	RenderSetting render_setting_;

protected:
	Texture* get_texture_at( uint_t n ) override { return n == 0 ? resolve_texture( texture_ ) : nullptr; }
	const Texture* get_texture_at( uint_t n ) const override { return n == 0 ? resolve_texture( texture_ ) : nullptr; }
	void set_texture_at( uint_t, Texture* t ) override { set_texture( t ); }

public:
	MatcapShader( GraphicsManager* graphics_manager, const char_t* input_layout_name = "main", const char_t* effect_technique_name = "matcap" )
		: BaseShader( graphics_manager )
		, render_setting_( graphics_manager, input_layout_name, effect_technique_name )
	{

	}

	void reload() override
	{
		render_setting_.reload();
	}

	MatcapShader* clone() const override { return new MatcapShader( *this ); }

	void set_texture( Texture* t ) override { texture_.set( * get_graphics_manager(), t ); }

	void bind() const override
	{
		get_game_constant_buffer()->bind_to_all();
		get_frame_constant_buffer()->bind_to_all();
		get_frame_drawing_constant_buffer()->bind_to_all();
		
		get_object_constant_buffer()->bind_to_vs();

		// スキニング用の定数バッファは、スキンメッシュ（バッファがあるとき）だけ結ぶ
		if ( get_skining_constant_buffer() )
		{
			get_skining_constant_buffer()->bind_to_vs();
		}

		bind_texture_to_ps( texture_, 3 );
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
