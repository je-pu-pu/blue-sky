#pragma once

#include "../BaseShader.h"
#include "../ShaderMixin.h"

namespace blue_sky::graphics::shader::post_effect
{

/**
 * ノイズポストエフェクト用シェーダー
 *
 */
class NoiseShader : public BaseShader, public ShaderMixin< NoiseShader, 0 >
{
private:
	Texture* get_texture_at( uint_t n ) override { return ShaderMixin::get_texture_at( n ); }
	const Texture* get_texture_at( uint_t n ) const override { return ShaderMixin::get_texture_at( n ); }
	void set_texture_at( uint_t n , Texture* t ) override { ShaderMixin::set_texture_at( n, t ); }

public:
	NoiseShader( const char_t* input_layout_name = "main", const char_t* effect_technique_name = "post_effect_noise" )
		: ShaderMixin( input_layout_name, effect_technique_name, { { ParameterType::INT, "seed" }, { ParameterType::FLOAT, "gain" } }, { ShaderStage::PS } )
	{

	}

	NoiseShader* clone() const override { return new NoiseShader( *this ); }

	void update() const override
	{
		ShaderMixin::update();
	}

	void bind() const override
	{
		ShaderMixin::bind();
	};

	void render( const Mesh* mesh, uint_t n ) const override
	{
		get_graphics_manager()->set_input_layout( input_layout_ );
		get_graphics_manager()->set_primitive_topology( PrimitiveTopology::TRIANGLE_LIST );
		get_graphics_manager()->render_technique( effect_technique_, [=] { bind(); mesh->render( n ); } );
	}
};


} // namespace blue_sky::graphics::shader::post_effect
