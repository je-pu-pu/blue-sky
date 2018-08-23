#pragma once

#include <blue_sky/ShaderResources.h>
#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/graphics/Direct3D11/MeshBuffer.h>

#include <memory>

class Direct3D11;
class Direct3D11Axis;

namespace blue_sky::graphics
{
	class SkyBox;
}

namespace blue_sky::graphics::direct_3d_11
{

/**
 * Direct3D 11 グラフィック管理クラス
 *
 */
class GraphicsManager : public blue_sky::graphics::GraphicsManager
{
public:
	typedef Direct3D11 Direct3D;

private:
	Direct3D*										direct_3d_;
	
	std::unique_ptr< SkyBox >						sky_box_;
	std::unique_ptr< Model >						ground_;
	
	std::unique_ptr< Direct3D11Axis >				debug_axis_;

	std::unique_ptr< GameConstantBuffer >			game_render_data_;
	std::unique_ptr< FrameConstantBuffer >			frame_render_data_;
	std::unique_ptr< FrameDrawingConstantBuffer >	frame_drawing_render_data_;
	std::unique_ptr< ObjectConstantBuffer >			shared_object_render_data_;

	std::unique_ptr< ObjectConstantBufferWithData >	sky_box_render_data_;
	std::unique_ptr< ObjectConstantBufferWithData > ground_render_data_;

protected:
	const InputLayout* get_input_layout( const char_t* ) const override;
	const EffectTechnique* get_effect_technique( const char_t* ) const override;

	void set_input_layout( const char_t* ) const override;
	void set_input_layout( const InputLayout* ) const override;
	void set_primitive_topology( PrimitiveTopology ) const override;

	[[nodiscard]] ShadowMap* create_shadow_map( uint_t, uint_t ) const override;
	[[nodiscard]] Line* create_line() const override;
	[[nodiscard]] Texture* load_texture_file( const char_t* ) const override;

	void render_debug_axis_model() const override;

public:
	explicit GraphicsManager( Direct3D* );
	virtual ~GraphicsManager();

	// void update() override;

	void set_default_viewport() override;
	void set_viewport( float_t x, float_t y, float_t w, float_t h, float_t min_d = 0.f, float_t max_d = 1.f ) override;

	MeshBuffer* create_mesh_buffer( MeshBuffer::Type type ) const override { return new MeshBuffer( direct_3d_, type ); }

	// void unload_mesh( const char_t* ) override;
	// void unload_mesh_all() override;

	// void unload_texture( const char_t* ) override;
	// void unload_texture_all() override;

	void set_sky_box( const char_t* ) override;
	void unset_sky_box() override;
	bool is_sky_box_set() const override;

	void set_ground( const char_t* ) override;
	void unset_ground() override;
	// bool is_ground_set() const override;

	void set_ambient_color( const Color& ) override;
	void set_shadow_color( const Color& ) override;
	void set_shadow_paper_color( const Color& ) override;
	void set_drawing_accent( float_t ) override;
	void set_drawing_line_type( int_t ) override;

	void set_eye_position( const Vector& );

	GameConstantBuffer* get_game_render_data() const override { return game_render_data_.get(); }
	FrameConstantBuffer* get_frame_render_data() const override { return frame_render_data_.get(); }
	FrameDrawingConstantBuffer* get_frame_drawing_render_data() const override { return frame_drawing_render_data_.get(); }
	ObjectConstantBuffer* get_shared_object_render_data() const override { return shared_object_render_data_.get(); }

	void setup_rendering() const override;
	void render_technique( const char_t*, const std::function< void () >& ) const override;
	void render_technique( const EffectTechnique*, const std::function< void () >& ) const override;
	void render_background() const override;

	/// @todo ちゃんと作る Font, size, 指定した Texture への描画
	void draw_text( float_t, float_t, float_t, float_t, const char_t*, const Color& ) const override;
	void draw_text_at_center( const char_t*, const Color& ) const override;

	void clear_debug_bullet() const override;
	void render_debug_bullet() const override;

}; // class GraphicsManager

} // namespace blue_sky::graphics::direct_3d_11
