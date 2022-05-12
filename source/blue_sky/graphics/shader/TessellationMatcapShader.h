#pragma once

#include <blue_sky/graphics/shader/BaseShader.h>

namespace blue_sky::graphics::shader
{

/**
 * Tessellation + Matcap シェーダー
 *
 */
template< typename Traits >
class BaseTessellationMatcapShader : public BaseShader
{
private:
	Texture* texture_ = nullptr;
	const InputLayout* input_layout_;
	const EffectTechnique* effect_technique_;

protected:
	Texture* get_texture() const { return texture_; }
	Texture* get_texture_at( uint_t ) override { return texture_; }
	const Texture* get_texture_at( uint_t ) const override { return texture_; }
	void set_texture_at( uint_t, Texture* t ) override { texture_ = t; }

public:
	BaseTessellationMatcapShader()
		: input_layout_( get_graphics_manager()->get_input_layout( Traits::input_layout_name ) )
		, effect_technique_( get_graphics_manager()->get_effect_technique( Traits::effect_technique_name ) )
	{

	}

	void set_texture( Texture* t ) { texture_ = t; }

	void render( const Mesh* mesh, uint_t n ) const override
	{
		get_graphics_manager()->set_input_layout( input_layout_ );
		get_graphics_manager()->set_primitive_topology( PrimitiveTopology::PATCH_LIST_3_CONTROL_POINT );
		get_graphics_manager()->render_technique( effect_technique_, [=] { bind(); mesh->render( n ); } );
	}
};

struct StaticTesselationMatcapTraits   { static constexpr const char* input_layout_name = "main"; static constexpr const char* effect_technique_name = "tessellation";      };
struct SkinningTesselationMatcapTraits { static constexpr const char* input_layout_name = "skin"; static constexpr const char* effect_technique_name = "tessellation_skin"; };

class TessellationMatcapShader : public BaseTessellationMatcapShader< StaticTesselationMatcapTraits >
{
public:
	TessellationMatcapShader* clone() const override { return new TessellationMatcapShader( *this ); }

	void bind() const override
	{
		// @todo 必要なものだけをバインドするようにする
		get_game_constant_buffer()->bind_to_all();
		get_frame_constant_buffer()->bind_to_all();
		get_frame_drawing_constant_buffer()->bind_to_all();
		
		get_graphics_manager()->load_texture( "media/model/balloon-disp2.png" )->bind_to_ds( 4 );
		get_graphics_manager()->load_texture( "media/model/balloon-norm.png"  )->bind_to_ps( 5 );

		get_object_constant_buffer()->bind_to_vs();
		get_object_constant_buffer()->bind_to_ds();

		get_object_constant_buffer()->bind_to_ps(); // for normal map + world

		if ( get_texture() )
		{
			get_texture()->bind_to_ps( 3 );
		}
	}
};

class SkinningTessellationMatcapShader : public BaseTessellationMatcapShader< SkinningTesselationMatcapTraits >
{
public:
	SkinningTessellationMatcapShader* clone() const override { return new SkinningTessellationMatcapShader( *this ); }

	void bind() const override
	{
		// @todo 必要なものだけをバインドするようにする
		get_game_constant_buffer()->bind_to_all();
		get_frame_constant_buffer()->bind_to_all();
		get_frame_drawing_constant_buffer()->bind_to_all();
		

		get_object_constant_buffer()->bind_to_vs();
		get_object_constant_buffer()->bind_to_ds();
		get_skining_constant_buffer()->bind_to_vs();

		if ( get_texture() )
		{
			get_texture()->bind_to_ps( 3 );
		}
	}
};

} // namespace blue_sky::graphics::shader
