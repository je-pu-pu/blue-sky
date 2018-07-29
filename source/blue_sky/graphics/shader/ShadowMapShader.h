#pragma once

#include <blue_sky/graphics/shader/BaseShader.h>

namespace blue_sky::graphics::shader
{

/**
 * シャドウマップ シェーダー
 *
 */
template< typename Traits >
class BaseShadowMapShader : public BaseShader
{
private:
	const InputLayout* input_layout_;
	const EffectTechnique* effect_technique_;

protected:
	Texture* get_texture_at( uint_t ) override { return nullptr; }
	const Texture* get_texture_at( uint_t ) const override { return nullptr; }
	void set_texture_at( uint_t, Texture* ) override { }

public:
	BaseShadowMapShader()
		: input_layout_( get_graphics_manager()->get_input_layout( Traits::input_layout_name ) )
		, effect_technique_( get_graphics_manager()->get_effect_technique( Traits::effect_technique_name ) )
	{

	}

	void render( const Mesh* mesh, uint_t n ) const override
	{
		get_graphics_manager()->set_input_layout( input_layout_ );
		get_graphics_manager()->set_primitive_topology( PrimitiveTopology::TRIANGLE_LIST );
		get_graphics_manager()->render_technique( effect_technique_, [=] { bind(); mesh->render( n ); } );
	}
};

struct ShadowMapShaderTraits         { static constexpr char* input_layout_name = "main"; static constexpr char* effect_technique_name = "shadow_map";      };
struct SkinningShadowMapShaderTraits { static constexpr char* input_layout_name = "skin"; static constexpr char* effect_technique_name = "shadow_map_skin"; };

class ShadowMapShader : public BaseShadowMapShader< ShadowMapShaderTraits >
{
public:
	ShadowMapShader* clone() const override { return new ShadowMapShader( *this ); }

	void bind() const override
	{
		get_frame_shader_resource()->bind_to_vs();
		
		get_object_shader_resource()->bind_to_vs();
		get_object_shader_resource()->bind_to_ds();
	}
};

class SkinningShadowMapShader : public BaseShadowMapShader< SkinningShadowMapShaderTraits >
{
public:
	SkinningShadowMapShader* clone() const override { return new SkinningShadowMapShader( *this ); }

	void bind() const override
	{
		get_frame_shader_resource()->bind_to_vs();
		
		get_object_shader_resource()->bind_to_vs();
		get_object_shader_resource()->bind_to_ds();

		get_skining_shader_resource()->bind_to_vs();
	}
};

} // namespace blue_sky::graphics::shader
