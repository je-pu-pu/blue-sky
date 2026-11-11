#pragma once

#include <blue_sky/graphics/shader/BaseShader.h>
#include <core/graphics/ShadowMap.h>

#include <optional>

namespace blue_sky::graphics::shader
{

/**
 * 陰影をつけないシェーダー
 *
 */
class FlatShader : public BaseShader
{
private:
	TextureSlot texture_;	///< 管理しているテクスチャはハンドルで持ち、描くたびに引く（TextureSlot）
	RenderSetting render_setting_;

	/// 同じモデルの物体をまとめて描くときのテクニック（作るときに名前を渡したときだけ。無ければまとめて描かない）
	std::optional< RenderSetting > instanced_render_setting_;

protected:
	Texture* get_texture_at( uint_t n ) override { return n == 0 ? resolve_texture( texture_ ) : nullptr; }
	const Texture* get_texture_at( uint_t n ) const override { return n == 0 ? resolve_texture( texture_ ) : nullptr; }
	void set_texture_at( uint_t, Texture* t ) override { set_texture( t ); }

	/// 物体の定数のほかに描くのに使うもの（定数バッファ・テクスチャ）を結ぶ
	virtual void bind_common() const
	{
		get_game_constant_buffer()->bind_to_all();
		get_frame_constant_buffer()->bind_to_all();
		get_frame_drawing_constant_buffer()->bind_to_all();

		// スキニング用の定数バッファは、スキンメッシュ（バッファがあるとき）だけ結ぶ
		if ( get_skining_constant_buffer() )
		{
			get_skining_constant_buffer()->bind_to_vs();
		}

		bind_texture_to_ps( texture_, 0 );

		get_graphics_manager()->bind_paper_texture();
	}

public:
	/**
	 * @param instanced_effect_technique_name 同じモデルの物体をまとめて描くときのテクニック（頂点シェーダーのほかは effect_technique_name と同じもの。nullptr ならまとめて描かない）
	 */
	FlatShader( GraphicsManager* graphics_manager, const char_t* input_layout_name = "main", const char_t* effect_technique_name = "flat", const char_t* instanced_effect_technique_name = nullptr )
		: BaseShader( graphics_manager )
		, texture_( * get_graphics_manager(), get_graphics_manager()->get_null_texture() )
		, render_setting_( graphics_manager, input_layout_name, effect_technique_name )
	{
		if ( instanced_effect_technique_name )
		{
			instanced_render_setting_.emplace( graphics_manager, input_layout_name, instanced_effect_technique_name );
		}
	}

	void reload() override
	{
		render_setting_.reload();

		if ( instanced_render_setting_ )
		{
			instanced_render_setting_->reload();
		}
	}

	FlatShader* clone() const override { return new FlatShader( *this ); }

	void set_texture( Texture* t ) override { texture_.set( * get_graphics_manager(), t ); }

	void bind() const override
	{
		bind_common();

		get_object_constant_buffer()->bind_to_vs();
	}

	void render( const Mesh* mesh, uint_t n ) const override
	{
		if ( is_texture_lost( texture_ ) )
		{
			return;
		}

		get_graphics_manager()->set_input_layout( render_setting_.get_input_layout() );
		get_graphics_manager()->set_primitive_topology( PrimitiveTopology::TRIANGLE_LIST );
		get_graphics_manager()->render_technique( render_setting_.get_effect_technique(), [=] { bind(); mesh->render( n ); } );
	}

	/// スキンメッシュ用でなく、まとめて描くテクニックを持つなら、まとめて描ける
	bool can_render_object_instances() const override { return instanced_render_setting_.has_value(); }

	/// render と同じ見た目を、物体の定数を呼ぶ側が結んだ入れ物（ObjectInstanceBuffer）から読んで、instance_count 個まとめて描く
	void render_object_instances( const Mesh* mesh, uint_t n, uint_t instance_count ) const override
	{
		if ( ! instanced_render_setting_ || is_texture_lost( texture_ ) )
		{
			return;
		}

		get_graphics_manager()->set_input_layout( instanced_render_setting_->get_input_layout() );
		get_graphics_manager()->set_primitive_topology( PrimitiveTopology::TRIANGLE_LIST );
		get_graphics_manager()->render_technique( instanced_render_setting_->get_effect_technique(), [=] { bind_common(); mesh->render_instanced( n, instance_count ); } );
	}
};

/**
 * 陰影をつけず影を付ける
 *
 */
class FlatShadowShader : public FlatShader
{
public:
	FlatShadowShader( GraphicsManager* graphics_manager, const char_t* input_layout_name = "main", const char_t* effect_technique_name = "flat", const char_t* instanced_effect_technique_name = nullptr )
		: FlatShader( graphics_manager, input_layout_name, effect_technique_name, instanced_effect_technique_name )
	{ }

	FlatShadowShader* clone() const override { return new FlatShadowShader( *this ); }

protected:
	/// FlatShader の分に、紙とシャドウマップを足す（結ぶ場所は物体の定数（b2）と重ならないので、結ぶ順は以前と違っても同じ状態になる）
	void bind_common() const override
	{
		FlatShader::bind_common();

		get_graphics_manager()->bind_paper_texture();
		get_graphics_manager()->get_shadow_map()->bind_to_render_scene();
	}
};

} // namespace blue_sky::graphics::shader
