#pragma once

#include <blue_sky/graphics/shader/BaseShader.h>

namespace blue_sky::graphics::shader
{

/**
 * フェーダー用シェーダー
 *
 */
class FaderShader : public BaseShader
{
private:
	const InputLayout* input_layout_;
	const EffectTechnique* effect_technique_;

protected:
	Texture* get_texture_at( uint_t ) override { return nullptr; }
	const Texture* get_texture_at( uint_t ) const override { return nullptr; }
	void set_texture_at( uint_t, Texture* ) override { }

public:
	FaderShader( const char_t* input_layout_name = "main", const char_t* effect_technique_name = "fader" )
		: input_layout_( get_graphics_manager()->get_input_layout( input_layout_name ) )
		, effect_technique_( get_graphics_manager()->get_effect_technique( effect_technique_name ) )
	{

	}

	FaderShader* clone() const override { return new FaderShader( *this ); }

	void bind() const override
	{
		get_object_constant_buffer()->bind_to_ps();
	}

	void render( const Mesh* mesh, uint_t n ) const override
	{
		get_graphics_manager()->set_input_layout( input_layout_ );
		get_graphics_manager()->set_primitive_topology( PrimitiveTopology::TRIANGLE_LIST );
		get_graphics_manager()->render_technique( effect_technique_, [=] { bind(); mesh->render( n ); } );
	}
};

} // namespace blue_sky::graphics::shader
