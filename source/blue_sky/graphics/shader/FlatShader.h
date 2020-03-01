#pragma once

#include <blue_sky/graphics/shader/BaseShader.h>
#include <core/graphics/ShadowMap.h>

namespace blue_sky::graphics::shader
{

/**
 * 陰影をつけないシェーダー
 *
 */
class FlatShader : public BaseShader
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
	FlatShader( const char_t* input_layout_name = "main", const char_t* effect_technique_name = "flat" )
		: input_layout_( get_graphics_manager()->get_input_layout( input_layout_name ) )
		, effect_technique_( get_graphics_manager()->get_effect_technique( effect_technique_name ) )
	{

	}

	FlatShader* clone() const override { return new FlatShader( *this ); }

	void set_texture( Texture* t ) { texture_ = t; }

	void bind() const override
	{
		/// @todo 必要なものだけをバインドするようにする
		get_game_constant_buffer()->bind_to_all();
		get_frame_constant_buffer()->bind_to_all();
		get_frame_drawing_constant_buffer()->bind_to_all();
		
		get_object_constant_buffer()->bind_to_vs();

		/// @todo スキニングありとなしでシェーダーの型を分ける？
		if ( get_skining_constant_buffer() )
		{
			get_skining_constant_buffer()->bind_to_vs();
		}

		/// @tood NullTexture 的なものを作って if をなくす
		if ( texture_ )
		{
			texture_->bind_to_ps( 0 );
		}
	}

	void render( const Mesh* mesh, uint_t n ) const override
	{
		get_graphics_manager()->set_input_layout( input_layout_ );
		get_graphics_manager()->set_primitive_topology( PrimitiveTopology::TRIANGLE_LIST );
		get_graphics_manager()->render_technique( effect_technique_, [=] { bind(); mesh->render( n ); } );
	}
};

/**
 * 陰影をつけず影を付ける
 *
 */
class FlatShadowShader : public FlatShader
{
public:
	FlatShadowShader( const char_t* input_layout_name = "main", const char_t* effect_technique_name = "flat" )
		: FlatShader( input_layout_name, effect_technique_name )
	{ }

	FlatShadowShader* clone() const override { return new FlatShadowShader( *this ); }

	void bind() const override
	{
		FlatShader::bind();

		get_graphics_manager()->bind_paper_texture();
		get_graphics_manager()->get_shadow_map()->bind_to_render_scene();
	}
};

} // namespace blue_sky::graphics::shader
