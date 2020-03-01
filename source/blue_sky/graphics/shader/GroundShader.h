#pragma once

#include <blue_sky/graphics/shader/BaseShader.h>

namespace blue_sky::graphics::shader
{

/**
 * �n�ʃV�F�[�_�[
 *
 */
class GroundShader : public BaseShader
{
private:
	Texture* texture_ = 0;
	const InputLayout* input_layout_;
	const EffectTechnique* effect_technique_;

protected:
	Texture* get_texture() const { return texture_; }
	Texture* get_texture_at( uint_t ) override { return texture_; }
	const Texture* get_texture_at( uint_t ) const override { return texture_; }
	void set_texture_at( uint_t, Texture* t ) override { texture_ = t; }

public:
	GroundShader()
		: input_layout_( get_graphics_manager()->get_input_layout( "main" ) )
		, effect_technique_( get_graphics_manager()->get_effect_technique( "ground" ) )
	{

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
		get_graphics_manager()->set_input_layout( input_layout_ );
		get_graphics_manager()->set_primitive_topology( PrimitiveTopology::TRIANGLE_LIST );
		get_graphics_manager()->render_technique( effect_technique_, [=] { bind(); mesh->render( n ); } );
	}
};

} // namespace blue_sky::graphics::shader
