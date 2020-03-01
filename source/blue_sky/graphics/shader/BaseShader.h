#pragma once

#include <GameMain.h>

#include <blue_sky/graphics/GraphicsManager.h>

#include <core/animation/AnimationPlayer.h>
#include <core/graphics/PrimitiveTopology.h>

#include <core/graphics/Model.h>
#include <core/graphics/Mesh.h>

namespace blue_sky::graphics::shader
{

/**
 * blue-sky シェーダー基底クラス
 *
 */
class BaseShader : public core::graphics::Shader
{
public:
	using Mesh				= core::graphics::Mesh;

	using PrimitiveTopology	= core::graphics::PrimitiveTopology;
	using InputLayout		= core::graphics::InputLayout;
	using EffectTechnique	= core::graphics::EffectTechnique;
	using ConstantBuffer	= core::graphics::ConstantBuffer;

protected:
	GraphicsManager* get_graphics_manager() const { return GameMain::get_instance()->get_graphics_manager(); }

	const ConstantBuffer* get_game_constant_buffer() const { return get_graphics_manager()->get_game_render_data(); }
	const ConstantBuffer* get_frame_constant_buffer() const { return get_graphics_manager()->get_frame_render_data(); }
	const ConstantBuffer* get_frame_drawing_constant_buffer() const { return get_graphics_manager()->get_frame_drawing_render_data(); }
	const ConstantBuffer* get_object_constant_buffer() const { return get_graphics_manager()->get_current_object_constant_buffer(); }
	const ConstantBuffer* get_skining_constant_buffer() const { return get_graphics_manager()->get_current_skinning_constant_buffer(); }

public:
	void render_model( const core::graphics::Model* m ) const override
	{
		m->get_mesh()->bind();

		for ( uint_t n = 0; n < m->get_mesh()->get_rendering_vertex_group_count(); n++ )
		{
			render( m->get_mesh(), n );
		}
	}
};

} // namespace blue_sky::graphics::shader
