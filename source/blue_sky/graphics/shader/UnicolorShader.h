#pragma once

#include "ShaderMixin.h"
#include <blue_sky/graphics/shader/BaseShader.h>
#include <core/graphics/ShadowMap.h>

namespace blue_sky::graphics::shader
{

/**
 * 単色でテクスチャも持たないシェーダー
 *
 */
class UnicolorShader : public BaseShader, public ShaderMixin< UnicolorShader, 10 >
{
public:

private:

protected:
	/// @todo xxx
	Texture* get_texture_at( uint_t ) override { return 0; }
	const Texture* get_texture_at( uint_t ) const override { return 0; }
	void set_texture_at( uint_t, Texture* ) override { }

public:
	UnicolorShader( const char_t* input_layout_name = "main", const char_t* effect_technique_name = "flat" )
		// : ShaderMixin( input_layout_name, effect_technique_name, { { ParameterType::COLOR, "color" } }, ShaderStage::VS | ShaderStage::PS )
		: ShaderMixin( input_layout_name, effect_technique_name, { { ParameterType::COLOR, "color" } }, { ShaderStage::VS, ShaderStage::PS } )
	{

	}

	UnicolorShader* clone() const override { return new UnicolorShader( *this ); }

	Color get_color( const char_t* name ) const override { return ShaderMixin::get_color( name ); }
	void set_color( const char_t* name, Color color ) override { ShaderMixin::set_color( name, color ); }

	void update() const override
	{
		ShaderMixin::update();
	}

	void bind() const override
	{
		/// @todo 必要なものだけをバインドするようにする
		get_game_constant_buffer()->bind_to_all();
		get_frame_constant_buffer()->bind_to_all();
		get_frame_drawing_constant_buffer()->bind_to_all();
		
		get_object_constant_buffer()->bind_to_vs();

		ShaderMixin::bind();
	};

	void render( const Mesh* mesh, uint_t n ) const override
	{
		get_graphics_manager()->set_input_layout( input_layout_ );
		get_graphics_manager()->set_primitive_topology( PrimitiveTopology::TRIANGLE_LIST );
		get_graphics_manager()->render_technique( effect_technique_, [=] { bind(); mesh->render( n ); } );
	}
};

} // namespace blue_sky::graphics::shader
