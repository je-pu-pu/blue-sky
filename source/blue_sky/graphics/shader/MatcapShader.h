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
	Texture* texture_ = 0;
	RenderSetting render_setting_;

protected:
	Texture* get_texture_at( uint_t ) override { return texture_; }
	const Texture* get_texture_at( uint_t ) const override { return texture_; }
	void set_texture_at( uint_t, Texture* t ) override { texture_ = t; }

public:
	MatcapShader( const char_t* input_layout_name = "main", const char_t* effect_technique_name = "matcap" )
		: render_setting_( input_layout_name, effect_technique_name )
	{

	}

	void reload() override
	{
		render_setting_.reload();
	}

	MatcapShader* clone() const override { return new MatcapShader( *this ); }

	void set_texture( Texture* t ) { texture_ = t; }

	void bind() const override
	{
		/// @todo 必要なものだけをバインドするようにする
		get_game_constant_buffer()->bind_to_all();
		get_frame_constant_buffer()->bind_to_all();
		get_frame_drawing_constant_buffer()->bind_to_all();
		
		get_object_constant_buffer()->bind_to_vs();

		/// @todo SkinningMatcapShader として分ける
		if ( get_skining_constant_buffer() )
		{
			get_skining_constant_buffer()->bind_to_vs();
		}

		if ( texture_ )
		{
			texture_->bind_to_ps( 3 );
		}
	}

	void render( const Mesh* mesh, uint_t n ) const override
	{
		get_graphics_manager()->set_input_layout( render_setting_.get_input_layout() );
		get_graphics_manager()->set_primitive_topology( PrimitiveTopology::TRIANGLE_LIST );
		get_graphics_manager()->render_technique( render_setting_.get_effect_technique(), [=] { bind(); mesh->render( n ); } );
	}
};

} // namespace blue_sky::graphics::shader
