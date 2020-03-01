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
	const InputLayout* input_layout_;
	const EffectTechnique* effect_technique_;

protected:
	Texture* get_texture_at( uint_t ) override { return texture_; }
	const Texture* get_texture_at( uint_t ) const override { return texture_; }
	void set_texture_at( uint_t, Texture* t ) override { texture_ = t; }

public:
	MatcapShader( const char_t* input_layout_name = "main", const char_t* effect_technique_name = "matcap" )
		: input_layout_( get_graphics_manager()->get_input_layout( input_layout_name ) )
		, effect_technique_( get_graphics_manager()->get_effect_technique( effect_technique_name ) )
	{

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
		get_graphics_manager()->set_input_layout( input_layout_ );
		get_graphics_manager()->set_primitive_topology( PrimitiveTopology::TRIANGLE_LIST );
		get_graphics_manager()->render_technique( effect_technique_, [=] { bind(); mesh->render( n ); } );
	}
};

} // namespace blue_sky::graphics::shader
