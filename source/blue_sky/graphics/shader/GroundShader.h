#pragma once

#include <blue_sky/graphics/shader/BaseShader.h>

namespace blue_sky::graphics::shader
{

/**
 * 地面シェーダー
 *
 */
class GroundShader : public BaseShader
{
private:
	Texture* texture_ = 0;
	RenderSetting render_setting_;

protected:
	Texture* get_texture() const { return texture_; }
	Texture* get_texture_at( uint_t ) override { return texture_; }
	const Texture* get_texture_at( uint_t ) const override { return texture_; }
	void set_texture_at( uint_t, Texture* t ) override { texture_ = t; }

public:
	GroundShader()
		: render_setting_( "main", "ground" )
	{

	}

	void reload() override
	{
		render_setting_.reload();
	}

	GroundShader* clone() const override { return new GroundShader( *this ); }

	void set_texture( Texture* t ) { texture_ = t; }

	void bind() const override
	{
		get_frame_constant_buffer()->bind_to_vs();
		get_object_constant_buffer()->bind_to_vs();
		get_object_constant_buffer()->bind_to_ps();
		get_texture()->bind_to_ps( 0 );
	}

	void render( const Mesh* mesh, uint_t n ) const override
	{
		get_graphics_manager()->set_input_layout( render_setting_.get_input_layout() );
		get_graphics_manager()->set_primitive_topology( PrimitiveTopology::TRIANGLE_LIST );
		get_graphics_manager()->render_technique( render_setting_.get_effect_technique(), [=] { bind(); mesh->render( n ); } );
	}
};

} // namespace blue_sky::graphics::shader
