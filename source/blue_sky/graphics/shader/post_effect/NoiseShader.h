#pragma once

#include "../BaseShader.h"
#include "../Shader.h"

namespace blue_sky::graphics::shader::post_effect
{

/**
 * ノイズポストエフェクト用シェーダー
 *
 */
class NoiseShader : public Shader< NoiseShader, 0 >
{
public:
	static inline const ParameterInfoList parameter_info_list = {
		{ ParameterType::INT,	"seed"         },
		{ ParameterType::FLOAT,	"gain",   0.1f },
		{ ParameterType::FLOAT,	"offset"       },
		{ ParameterType::FLOAT,	"size",   5.f  },
		{ ParameterType::FLOAT,	"speed",  5.f  },
	};

public:
	NoiseShader( const char_t* input_layout_name = "main", const char_t* effect_technique_name = "post_effect_noise" )
		: Shader( input_layout_name, effect_technique_name, { ShaderStage::PS } )
	{

	}

	NoiseShader* clone() const override { return new NoiseShader( *this ); }

	void render( const Mesh* mesh, uint_t n ) const override
	{
		get_graphics_manager()->set_input_layout( input_layout_ );
		get_graphics_manager()->set_primitive_topology( PrimitiveTopology::TRIANGLE_LIST );
		get_graphics_manager()->render_technique( effect_technique_, [=] { bind(); mesh->render( n ); } );
	}
};


} // namespace blue_sky::graphics::shader::post_effect
