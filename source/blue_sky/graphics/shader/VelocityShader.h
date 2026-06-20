#pragma once

#include <blue_sky/graphics/shader/BaseShader.h>

namespace blue_sky::graphics::shader
{

/**
 * モーションベクトル ( 速度 ) を書き出すシェーダー ( 案B / 段階0b )
 *
 * 現フレームと前フレームの画面位置の差 ( NDC 速度 ) を RG へ出力する。
 * 前フレーム行列は ObjectConstantBuffer.PrevWorld / FrameConstantBuffer.PrevView / PrevProjection を使う。
 * 入力レイアウトは静的メッシュ用の "main" ( COMMON_POS_NORM_UV )。スキンメッシュは未対応。
 */
class VelocityShader : public BaseShader
{
private:
	RenderSetting render_setting_;

protected:
	Texture* get_texture_at( uint_t ) override { return nullptr; }
	const Texture* get_texture_at( uint_t ) const override { return nullptr; }
	void set_texture_at( uint_t, Texture* ) override { }

public:
	VelocityShader( const char_t* input_layout_name = "main", const char_t* effect_technique_name = "velocity" )
		: render_setting_( input_layout_name, effect_technique_name )
	{ }

	void reload() override
	{
		render_setting_.reload();
	}

	VelocityShader* clone() const override { return new VelocityShader( *this ); }

	void bind() const override
	{
		// 前フレーム行列を含む Frame / Object 定数バッファを VS で使えるようにする
		get_frame_constant_buffer()->bind_to_all();
		get_object_constant_buffer()->bind_to_vs();

		if ( get_skining_constant_buffer() )
		{
			get_skining_constant_buffer()->bind_to_vs();
		}
	}

	void render( const Mesh* mesh, uint_t n ) const override
	{
		get_graphics_manager()->set_input_layout( render_setting_.get_input_layout() );
		get_graphics_manager()->set_primitive_topology( PrimitiveTopology::TRIANGLE_LIST );
		get_graphics_manager()->render_technique( render_setting_.get_effect_technique(), [=] { bind(); mesh->render( n ); } );
	}
};

} // namespace blue_sky::graphics::shader
