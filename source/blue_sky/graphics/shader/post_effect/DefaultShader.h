#pragma once

#include <blue_sky/graphics/shader/BaseShader.h>
#include <core/graphics/ShadowMap.h>

namespace blue_sky::graphics::shader::post_effect
{

/**
 * �f�t�H���g�̃|�X�g�G�t�F�N�g�p�V�F�[�_�[
 *
 */
class DefaultShader : public BaseShader
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
	DefaultShader( const char_t* input_layout_name = "main", const char_t* effect_technique_name = "post_effect_default" )
		: input_layout_( get_graphics_manager()->get_input_layout( input_layout_name ) )
		, effect_technique_( get_graphics_manager()->get_effect_technique( effect_technique_name ) )
	{

	}

	DefaultShader* clone() const override { return new DefaultShader( *this ); }

	void set_texture( Texture* t ) { texture_ = t; }

	void bind() const override
	{
		/// @todo �K�v�Ȃ��̂������o�C���h����悤�ɂ���
		get_game_shader_resource()->bind_to_all();
		get_frame_shader_resource()->bind_to_all();
		get_frame_drawing_shader_resource()->bind_to_all();
		
		get_object_shader_resource()->bind_to_vs();

		texture_->bind_to_ps( 0 );
	}

	void render( const Mesh* mesh, uint_t n ) const override
	{
		get_graphics_manager()->set_input_layout( input_layout_ );
		get_graphics_manager()->set_primitive_topology( PrimitiveTopology::TRIANGLE_LIST );
		get_graphics_manager()->render_technique( effect_technique_, [=] { bind(); mesh->render( n ); } );
	}
};


} // namespace blue_sky::graphics::shader::post_effect
