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
	RenderSetting render_setting_;

protected:
	Texture* get_texture_at( uint_t ) override { return nullptr; }
	const Texture* get_texture_at( uint_t ) const override { return nullptr; }
	void set_texture_at( uint_t, Texture* ) override { }

public:
	FaderShader( const char_t* input_layout_name = "main", const char_t* effect_technique_name = "fader" )
		: render_setting_( input_layout_name, effect_technique_name )
	{

	}

	void reload() override
	{
		render_setting_.reload();
	}

	FaderShader* clone() const override { return new FaderShader( *this ); }

	void bind() const override
	{
		get_object_constant_buffer()->bind_to_ps();
	}

	void render( const Mesh* mesh, uint_t n ) const override
	{
		get_graphics_manager()->set_input_layout( render_setting_.get_input_layout() );
		get_graphics_manager()->set_primitive_topology( PrimitiveTopology::TRIANGLE_LIST );
		get_graphics_manager()->render_technique( render_setting_.get_effect_technique(), [=] { bind(); mesh->render( n ); } );
	}
};

} // namespace blue_sky::graphics::shader
