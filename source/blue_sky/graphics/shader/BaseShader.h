#pragma once

#include <blue_sky/GameMain.h>

#include <blue_sky/graphics/GraphicsManager.h>

#include <core/animation/AnimationPlayer.h>
#include <core/graphics/PrimitiveTopology.h>

#include <core/graphics/Model.h>
#include <core/graphics/Mesh.h>

#include <core/math/Vector.h>

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

	float_t get_scalar( const char_t* ) const override { return 0.f; }
	float_t get_scalar_at( uint_t ) const override { return 0.f; }

	void set_scalar( const char_t*, float_t ) override { }
	void set_scalar_at( uint_t, float_t ) override { }

	Vector get_vector( const char_t* ) const override { return Vector::Zero; }
	Vector get_vector_at( uint_t ) const override { return Vector::Zero; }

	void set_vector( const char_t*, Vector ) override { }
	void set_vector_at( uint_t, Vector ) override { }

	Color get_color( const char_t* ) const override { return Color::Black; }
	Color get_color_at( uint_t ) const override { return Color::Black; }

	void set_color( const char_t*, Color ) override { }
	void set_color_at( uint_t, Color ) override { }

	Texture* get_texture( const char_t* ) override { return nullptr; }
	const Texture* get_texture( const char_t* ) const override { return nullptr; }

	void set_texture( const char_t*, Texture* ) override { }

public:
	void update() const override { }

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
