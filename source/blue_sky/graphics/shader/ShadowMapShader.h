#pragma once

#include <blue_sky/graphics/shader/BaseShader.h>

namespace blue_sky::graphics::shader
{

/**
 * シャドウマップ シェーダー
 *
 */
// template< typename Traits >
class BaseShadowMapShader : public BaseShader
{
private:
	RenderSetting render_setting_;
	const ConstantBuffer* constant_buffer_ = nullptr;

protected:
	Texture* get_texture_at( uint_t ) override { return nullptr; }
	const Texture* get_texture_at( uint_t ) const override { return nullptr; }
	void set_texture_at( uint_t, Texture* ) override { }

	const ConstantBuffer* get_constant_buffer() const { return constant_buffer_; }

public:
	BaseShadowMapShader( const char_t* input_layout_name, const char_t* effect_technique_name )
		: render_setting_( input_layout_name, effect_technique_name )
	{

	}

	void reload() override
	{
		render_setting_.reload();
	}

	void set_constant_buffer( const ConstantBuffer* r ) { constant_buffer_ = r; }

	void render( const Mesh* mesh, uint_t n ) const override
	{
		get_graphics_manager()->set_input_layout( render_setting_.get_input_layout() );
		get_graphics_manager()->set_primitive_topology( PrimitiveTopology::TRIANGLE_LIST );
		get_graphics_manager()->render_technique( render_setting_.get_effect_technique(), [=] { bind(); mesh->render( n ); } );
	}
};

struct ShadowMapShaderTraits         { static constexpr const char* input_layout_name = "main"; static constexpr const char* effect_technique_name = "shadow_map";      };
struct SkinningShadowMapShaderTraits { static constexpr const char* input_layout_name = "skin"; static constexpr const char* effect_technique_name = "shadow_map_skin"; };

class ShadowMapShader : public BaseShadowMapShader // < ShadowMapShaderTraits >
{
public:
	ShadowMapShader()
		: BaseShadowMapShader( "main", "shadow_map" )
	{ }

	ShadowMapShader* clone() const override { return new ShadowMapShader( *this ); }

	void bind() const override
	{
		get_frame_constant_buffer()->bind_to_vs();
		
		get_object_constant_buffer()->bind_to_vs();
		get_object_constant_buffer()->bind_to_ds();

		get_constant_buffer()->bind_to_vs();
	}
};

class SkinningShadowMapShader : public BaseShadowMapShader // < SkinningShadowMapShaderTraits >
{
public:
	SkinningShadowMapShader()
		: BaseShadowMapShader( "skin", "shadow_map_skin" )
	{ }

	SkinningShadowMapShader* clone() const override { return new SkinningShadowMapShader( *this ); }

	void bind() const override
	{
		get_frame_constant_buffer()->bind_to_vs();
		
		get_object_constant_buffer()->bind_to_vs();
		get_object_constant_buffer()->bind_to_ds();

		get_skining_constant_buffer()->bind_to_vs();

		get_constant_buffer()->bind_to_vs();
	}
};

} // namespace blue_sky::graphics::shader
