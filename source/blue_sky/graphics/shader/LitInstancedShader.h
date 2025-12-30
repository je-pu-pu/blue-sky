#pragma once

#include <blue_sky/graphics/shader/BaseShader.h>
#include <blue_sky/graphics/Mesh.h>
#include <core/graphics/Direct3D11/InstanceBuffer.h>

namespace blue_sky::graphics::shader
{

using BlueSkyMesh = blue_sky::graphics::Mesh;
using InstanceBuffer = core::graphics::direct_3d_11::InstanceBuffer;

/**
 * インスタンシング対応のシェーダー
 *
 */
class LitInstancedShader : public BaseShader
{
private:
	Texture* texture_ = nullptr;
	RenderSetting render_setting_;

protected:
	Texture* get_texture_at( uint_t ) override { return texture_; }
	const Texture* get_texture_at( uint_t ) const override { return texture_; }
	void set_texture_at( uint_t, Texture* t ) override { texture_ = t; }

public:
	LitInstancedShader( const char_t* input_layout_name = "main", const char_t* effect_technique_name = "lit_instanced" )
		: texture_( get_graphics_manager()->get_null_texture() )
		, render_setting_( input_layout_name, effect_technique_name )
	{

	}

	void reload() override
	{
		render_setting_.reload();
	}

	LitInstancedShader* clone() const override { return new LitInstancedShader( *this ); }

	void set_texture( Texture* t ) { texture_ = t; }

	void bind() const override
	{
		get_game_constant_buffer()->bind_to_all();
		get_frame_constant_buffer()->bind_to_all();
		get_frame_drawing_constant_buffer()->bind_to_all();

		// インスタンシングではオブジェクト定数バッファは使用しない
		// (ワールド行列はStructuredBufferから読み込む)

		texture_->bind_to_ps( 0 );

		get_graphics_manager()->bind_paper_texture();
	}

	void render( const Mesh* mesh, uint_t n ) const override
	{
		get_graphics_manager()->set_input_layout( render_setting_.get_input_layout() );
		get_graphics_manager()->set_primitive_topology( PrimitiveTopology::TRIANGLE_LIST );
		get_graphics_manager()->render_technique( render_setting_.get_effect_technique(), [=] { bind(); mesh->render( n ); } );
	}

	/**
	 * インスタンシング描画
	 */
	void render_instanced( const BlueSkyMesh* mesh, uint_t n, uint_t instance_count, const InstanceBuffer* instance_buffer ) const
	{
		get_graphics_manager()->set_input_layout( render_setting_.get_input_layout() );
		get_graphics_manager()->set_primitive_topology( PrimitiveTopology::TRIANGLE_LIST );
		get_graphics_manager()->render_technique( render_setting_.get_effect_technique(), [=] {
			bind();
			mesh->bind();
			instance_buffer->bind_to_vs();  // pass->apply() の後でバインド
			mesh->render_instanced( n, instance_count );
		} );
	}
};

} // namespace blue_sky::graphics::shader
