#pragma once

#include "Shader.h"

namespace blue_sky::graphics::shader
{

/**
 * ポイントスプライトシェーダー
 *
 */
class PointSpriteShader : public Shader< PointSpriteShader >
{
public:
	static inline const ParameterInfoList parameter_info_list = {};

protected:

public:
	PointSpriteShader( const char_t* input_layout_name = "pos_norm", const char_t* effect_technique_name = "point_sprite" )
		: Shader( input_layout_name, effect_technique_name, { ShaderStage::VS, ShaderStage::PS } )
	{

	}

	PointSpriteShader* clone() const override { return new PointSpriteShader( *this ); }

	void bind() const override
	{
		Shader::bind();

		get_game_constant_buffer()->bind_to_gs();	// 画面の幅、高さを GS で利用する
		get_frame_constant_buffer()->bind_to_vs();	// View, Projection を VS で利用する
		get_frame_constant_buffer()->bind_to_gs();	// View, Projection を GS で利用する
		// get_object_constant_buffer()->bind_to_vs();	// World を VS で利用する
	};

	void render( const Mesh* mesh, uint_t n ) const override
	{
		get_graphics_manager()->set_input_layout( render_setting_.get_input_layout() );
		get_graphics_manager()->set_primitive_topology( PrimitiveTopology::POINT_LIST );
		get_graphics_manager()->render_technique( render_setting_.get_effect_technique(), [=] { bind(); mesh->render( n ); } );
	}
};

} // namespace blue_sky::graphics::shader
