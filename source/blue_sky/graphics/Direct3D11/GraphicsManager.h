#pragma once

#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/ShaderResources.h>

#include <memory>

class Direct3D11;
class Direct3D11SkyBox;
class Direct3D11Axis;

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
	Direct3D* direct_3d_;
	
	std::unique_ptr< Direct3D11SkyBox >				sky_box_;
	std::unique_ptr< Mesh >							ground_;

	std::unique_ptr< Direct3D11Axis >				debug_axis_;

	std::unique_ptr< GameConstantBuffer >			game_render_data_;
	std::unique_ptr< FrameConstantBuffer >			frame_render_data_;
	std::unique_ptr< FrameDrawingConstantBuffer >	frame_drawing_render_data_;
	std::unique_ptr< ObjectConstantBuffer >			shared_object_render_data_;

	std::unique_ptr< ObjectConstantBufferWithData >	sky_box_render_data_;
	std::unique_ptr< ObjectConstantBufferWithData > ground_render_data_;

	bool is_fading_in_ = true;
	float_t fade_speed_ = 0.f;

protected:
	const InputLayout* get_input_layout( const char_t* ) const override;
	const EffectTechnique* get_effect_technique( const char_t* ) const override;

	void set_input_layout( const char_t* ) const override;
	void set_input_layout( const InputLayout* ) const override;
	void set_primitive_topology( PrimitiveTopology ) const override;

	void render_debug_axis_model() const override;

public:
	GraphicsManager( Direct3D* );
	virtual ~GraphicsManager();

	void update() override;

	Mesh* load_mesh( const char_t*, const char_t* ) override;
	// Mesh* get_mesh( const char_t* ) override;

	void unload_mesh( const char_t* ) override;
	void unload_mesh_all() override;

	DrawingMesh* create_drawing_mesh() override;
	DrawingLine* create_drawing_line() override;

	Texture* load_texture( const char_t*, const char_t* ) override;
	Texture* get_texture( const char_t* ) override;

	void unload_texture( const char_t* ) override;
	void unload_texture_all() override;

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

	void set_eye_position( const Vector3& );

	GameConstantBuffer* get_game_render_data() const override { return game_render_data_.get(); }
	FrameConstantBuffer* get_frame_render_data() const override { return frame_render_data_.get(); }
	FrameDrawingConstantBuffer* get_frame_drawing_render_data() const override { return frame_drawing_render_data_.get(); }
	ObjectConstantBuffer* get_shared_object_render_data() const override { return shared_object_render_data_.get(); }

	void setup_rendering() const override;
	void render_technique( const char_t*, const std::function< void () >& ) const override;
	void render_technique( const EffectTechnique*, const std::function< void () >& ) const override;
	void render_background() const override;

	void set_fade_color( const Color& ) override;

	void start_fade_in( float_t ) override;
	void start_fade_out( float_t ) override;

	void fade_in( float_t speed = 1.f ) override;
	void fade_out( float_t speed = 1.f ) override;

	void render_fader() const override;

	/// @todo ちゃんと作る Font, size, 指定した Texture への描画
	void draw_text( float_t, float_t, float_t, float_t, const char_t*, const Color& ) const override;
	void draw_text_at_center( const char_t*, const Color& ) const override;

	void clear_debug_bullet() const override;
	void render_debug_bullet() const override;

}; // class GraphicsManager

} // namespace blue_sky::graphics::direct_3d_11
