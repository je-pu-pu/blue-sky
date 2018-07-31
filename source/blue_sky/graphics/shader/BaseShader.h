#pragma once

#include <GameMain.h>
#include <AnimationPlayer.h>
#include <blue_sky/graphics/GraphicsManager.h>

#include <core/graphics/PrimitiveTopology.h>

#include <game/Model.h>
#include <game/Mesh.h>
#include <game/Shader.h>
#include <game/Texture.h>

namespace blue_sky::graphics::shader
{

/**
 * blue-sky シェーダー基底クラス
 *
 */
class BaseShader : public game::Shader
{
public:
	typedef game::Mesh Mesh;
	typedef game::Texture Texture;

	typedef core::graphics::PrimitiveTopology PrimitiveTopology;
	typedef core::graphics::InputLayout InputLayout;
	typedef core::graphics::EffectTechnique EffectTechnique;

protected:
	GraphicsManager* get_graphics_manager() const { return GameMain::get_instance()->get_graphics_manager(); }

	const ShaderResource* get_game_shader_resource() const { return get_graphics_manager()->get_game_render_data(); }
	const ShaderResource* get_frame_shader_resource() const { return get_graphics_manager()->get_frame_render_data(); }
	const ShaderResource* get_frame_drawing_shader_resource() const { return get_graphics_manager()->get_frame_drawing_render_data(); }
	const ShaderResource* get_object_shader_resource() const { return get_graphics_manager()->get_current_object_shader_resource(); }
	const ShaderResource* get_skining_shader_resource() const { return get_graphics_manager()->get_current_skinning_shader_resource(); }

public:
	void render_model( const game::Model* m ) const override
	{
		for ( uint_t n = 0; n < m->get_mesh()->get_rendering_vertex_group_count(); n++ )
		{
			render( m->get_mesh(), n );
		}
	}
};

} // namespace blue_sky::graphics::shader
