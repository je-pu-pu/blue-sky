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
	BaseShadowMapShader( GraphicsManager* graphics_manager, const char_t* input_layout_name, const char_t* effect_technique_name )
		: BaseShader( graphics_manager )
		, render_setting_( graphics_manager, input_layout_name, effect_technique_name )
	{

	}

	void reload() override
	{
		render_setting_.reload();
	}

	void set_constant_buffer( const ConstantBuffer* r ) { constant_buffer_ = r; }

	/// 物体の線を instance_count 個まとめて描く（できるシェーダーだけ。既定は何もしない）
	virtual void render_line_instances( const graphics::Line*, uint_t ) const { }

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
private:
	RenderSetting instanced_render_setting_;		///< 同じモデルの物体をまとめて描くとき（頂点シェーダーのほかは shadow_map と同じ）
	RenderSetting instanced_line_render_setting_;	///< 物体の線をまとめて描くとき（入力レイアウト "pos_color"）

	/// 物体の定数のほかに使うもの（フレームの定数・シャドウマップの行列）を結ぶ
	void bind_common() const
	{
		get_frame_constant_buffer()->bind_to_vs();
		get_constant_buffer()->bind_to_vs();
	}

public:
	ShadowMapShader( GraphicsManager* graphics_manager )
		: BaseShadowMapShader( graphics_manager, "main", "shadow_map" )
		, instanced_render_setting_( graphics_manager, "main", "shadow_map_instanced" )
		, instanced_line_render_setting_( graphics_manager, "pos_color", "shadow_map_line_instanced" )
	{ }

	ShadowMapShader* clone() const override { return new ShadowMapShader( *this ); }

	void reload() override
	{
		BaseShadowMapShader::reload();

		instanced_render_setting_.reload();
		instanced_line_render_setting_.reload();
	}

	void bind() const override
	{
		get_frame_constant_buffer()->bind_to_vs();

		get_object_constant_buffer()->bind_to_vs();
		get_object_constant_buffer()->bind_to_ds();

		get_constant_buffer()->bind_to_vs();
	}

	bool can_render_object_instances() const override { return true; }

	/// render と同じものを、物体の定数を呼ぶ側が結んだ入れ物から読んで、instance_count 個まとめて描く
	void render_object_instances( const Mesh* mesh, uint_t n, uint_t instance_count ) const override
	{
		get_graphics_manager()->set_input_layout( instanced_render_setting_.get_input_layout() );
		get_graphics_manager()->set_primitive_topology( PrimitiveTopology::TRIANGLE_LIST );
		get_graphics_manager()->render_technique( instanced_render_setting_.get_effect_technique(), [=] { bind_common(); mesh->render_instanced( n, instance_count ); } );
	}

	/// 線を instance_count 個まとめて描く（物体の定数は呼ぶ側が結んでおく。線分のまま描く）
	void render_line_instances( const graphics::Line* line, uint_t instance_count ) const override
	{
		get_graphics_manager()->set_input_layout( instanced_line_render_setting_.get_input_layout() );
		get_graphics_manager()->render_technique( instanced_line_render_setting_.get_effect_technique(), [=] { bind_common(); line->render_instanced( instance_count ); } );
	}
};

class SkinningShadowMapShader : public BaseShadowMapShader // < SkinningShadowMapShaderTraits >
{
public:
	SkinningShadowMapShader( GraphicsManager* graphics_manager )
		: BaseShadowMapShader( graphics_manager, "skin", "shadow_map_skin" )
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
