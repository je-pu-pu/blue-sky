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
	TextureSlot texture_;	///< 管理しているテクスチャはハンドルで持ち、描くたびに引く（TextureSlot）
	RenderSetting render_setting_;

protected:
	Texture* get_texture() const { return resolve_texture( texture_ ); }
	const TextureSlot& get_texture_slot() const { return texture_; }
	Texture* get_texture_at( uint_t n ) override { return n == 0 ? resolve_texture( texture_ ) : nullptr; }
	const Texture* get_texture_at( uint_t n ) const override { return n == 0 ? resolve_texture( texture_ ) : nullptr; }
	void set_texture_at( uint_t, Texture* t ) override { set_texture( t ); }

public:
	BaseTessellationMatcapShader( GraphicsManager* graphics_manager )
		: BaseShader( graphics_manager )
		, render_setting_( graphics_manager, Traits::input_layout_name, Traits::effect_technique_name )
	{

	}

	void reload() override
	{
		render_setting_.reload();
	}

	void set_texture( Texture* t ) override { texture_.set( * get_graphics_manager(), t ); }

	void render( const Mesh* mesh, uint_t n ) const override
	{
		if ( is_texture_lost( texture_ ) )
		{
			return;
		}

		get_graphics_manager()->set_input_layout( render_setting_.get_input_layout() );
		get_graphics_manager()->set_primitive_topology( PrimitiveTopology::PATCH_LIST_3_CONTROL_POINT );
		get_graphics_manager()->render_technique( render_setting_.get_effect_technique(), [=] { bind(); mesh->render( n ); } );
	}
};

struct StaticTesselationMatcapTraits   { static constexpr const char* input_layout_name = "main"; static constexpr const char* effect_technique_name = "tessellation";      };
struct SkinningTesselationMatcapTraits { static constexpr const char* input_layout_name = "skin"; static constexpr const char* effect_technique_name = "tessellation_skin"; };

class TessellationMatcapShader : public BaseTessellationMatcapShader< StaticTesselationMatcapTraits >
{
public:
	using BaseTessellationMatcapShader::BaseTessellationMatcapShader;

	TessellationMatcapShader* clone() const override { return new TessellationMatcapShader( *this ); }

	void bind() const override
	{
		get_game_constant_buffer()->bind_to_all();
		get_frame_constant_buffer()->bind_to_all();
		get_frame_drawing_constant_buffer()->bind_to_all();
		
		get_graphics_manager()->load_texture( "media/model/balloon-disp2.png" )->bind_to_ds( 4 );
		get_graphics_manager()->load_texture( "media/model/balloon-norm.png"  )->bind_to_ps( 5 );

		get_object_constant_buffer()->bind_to_vs();
		get_object_constant_buffer()->bind_to_ds();

		get_object_constant_buffer()->bind_to_ps(); // for normal map + world

		bind_texture_to_ps( get_texture_slot(), 3 );
	}
};

class SkinningTessellationMatcapShader : public BaseTessellationMatcapShader< SkinningTesselationMatcapTraits >
{
public:
	using BaseTessellationMatcapShader::BaseTessellationMatcapShader;

	SkinningTessellationMatcapShader* clone() const override { return new SkinningTessellationMatcapShader( *this ); }

	void bind() const override
	{
		get_game_constant_buffer()->bind_to_all();
		get_frame_constant_buffer()->bind_to_all();
		get_frame_drawing_constant_buffer()->bind_to_all();
		

		get_object_constant_buffer()->bind_to_vs();
		get_object_constant_buffer()->bind_to_ds();
		get_skining_constant_buffer()->bind_to_vs();

		bind_texture_to_ps( get_texture_slot(), 3 );
	}
};

} // namespace blue_sky::graphics::shader
