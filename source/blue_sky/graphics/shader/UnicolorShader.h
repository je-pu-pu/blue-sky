#pragma once

#include "Shader.h"
#include <blue_sky/graphics/shader/BaseShader.h>
#include <core/graphics/ShadowMap.h>

namespace blue_sky::graphics::shader
{

/**
 * 単色でテクスチャも持たないシェーダー
 *
 */
class UnicolorShader : public Shader< UnicolorShader, 10 >
{
public:
	static inline const ParameterInfoList parameter_info_list = { { ParameterType::COLOR, "color" } };

protected:

public:
	UnicolorShader( const char_t* input_layout_name = "main", const char_t* effect_technique_name = "flat" )
		: Shader( input_layout_name, effect_technique_name, { ShaderStage::VS, ShaderStage::PS } )
	{

	}

	UnicolorShader* clone() const override { return new UnicolorShader( *this ); }

	void bind() const override
	{
		/// @todo 必要なものだけをバインドするようにする
		get_game_constant_buffer()->bind_to_all();
		get_frame_constant_buffer()->bind_to_all();
		get_frame_drawing_constant_buffer()->bind_to_all();
		
		get_object_constant_buffer()->bind_to_vs();

		Shader::bind();
	};

	void render( const Mesh* mesh, uint_t n ) const override
	{
		get_graphics_manager()->set_input_layout( input_layout_ );
		get_graphics_manager()->set_primitive_topology( PrimitiveTopology::TRIANGLE_LIST );
		get_graphics_manager()->render_technique( effect_technique_, [=] { bind(); mesh->render( n ); } );
	}
};

} // namespace blue_sky::graphics::shader
