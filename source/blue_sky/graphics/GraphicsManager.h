#pragma once

#include <blue_sky/graphics/Model.h>
#include <game/Mesh.h>

#include <blue_sky/ShaderResources.h>

#include <core/graphics/PrimitiveTopology.h>
#include <core/type.h>

#include <game/GraphicsManager.h>
#include <game/ResourceManager.h>
#include <game/Shader.h>

#include <common/safe_ptr.h>
#include <common/auto_ptr.h>

#include <vector>
#include <functional>

// class Vector3;
class SkinningAnimationSet;
class FbxFileLoader;

namespace game
{

class Mesh;
class Shader;
class Texture;

}

namespace core::graphics
{

class InputLayout;
class EffectTechnique;

}

namespace blue_sky
{

class DrawingMesh;
class DrawingLine;

class ActiveObject;
class ActiveObjectManager;

namespace graphics
{


/**
 * グラフィック管理クラス
 *
 */
class GraphicsManager : public game::GraphicsManager
{
public:
	typedef Model Model;
	typedef game::Mesh Mesh;
	typedef game::Shader Shader;
	typedef game::Texture Texture;

	typedef game::ResourceManager< Model > ModelManager;
	typedef game::ResourceManager< Mesh > MeshManager;
	typedef game::ResourceManager< Shader > ShaderManager;

	typedef core::graphics::PrimitiveTopology PrimitiveTopology;
	typedef core::graphics::InputLayout InputLayout;
	typedef core::graphics::EffectTechnique EffectTechnique;

private:
	std::vector< Texture* >		paper_texture_list_;
	Texture*					paper_texture_ = 0;

	ModelManager				model_manager_;
	MeshManager					mesh_manager_;
	ShaderManager				shader_manager_;

	mutable const ShaderResource* current_object_shader_resource_ = 0;
	mutable const ShaderResource* current_skinning_shader_resource_ = 0;

	common::auto_ptr< FbxFileLoader> fbx_file_loader_;
	bool is_debug_axis_enabled_ = true;

protected:
	ShaderManager* get_shader_manager() { return & shader_manager_; }

	void render_debug_axis_for_bones( const ActiveObject* ) const;
	virtual void render_debug_axis_model() const = 0;

public:
	GraphicsManager();
	virtual ~GraphicsManager();

	virtual void update() = 0;

	virtual Mesh* load_mesh( const char_t*, const char_t* ) = 0;
	// virtual Mesh* get_mesh( const char_t* ) = 0;

	virtual DrawingMesh* create_drawing_mesh() = 0;
	virtual DrawingLine* create_drawing_line() = 0;

	DrawingMesh* load_drawing_mesh( const char_t*, common::safe_ptr< SkinningAnimationSet >& );
	DrawingLine* load_drawing_line( const char_t* );
	
	// template< typename Type=Model > Model* get_model( const char_t* name ) { return model_manager_.get< Type >( name ); }

	template< typename Type > Type* create_shader( const char_t* name ) { return shader_manager_.create< Type >( name ); }
	template< typename Type=Shader > Type* get_shader( const char_t* name ) { return shader_manager_.get< Type >( name ); }

	virtual Texture* load_texture( const char_t*, const char_t* ) = 0;
	virtual Texture* get_texture( const char_t* ) = 0;

	void setup_default_shaders();
	void load_paper_textures();

	virtual void unload_mesh( const char_t* ) = 0;
	virtual void unload_mesh_all() = 0;

	virtual void unload_texture( const char_t* ) = 0;
	virtual void unload_texture_all() = 0;

	void set_paper_texture_type( int_t );
	void bind_paper_texture() const;

	virtual void set_sky_box( const char_t* ) = 0;
	virtual void unset_sky_box() = 0;
	virtual bool is_sky_box_set() const = 0;

	virtual void set_ground( const char_t* ) = 0;
	virtual void unset_ground() = 0;
	// virtual bool is_ground_set() const = 0;

	virtual void set_ambient_color( const Color& ) = 0;
	virtual void set_shadow_color( const Color& ) = 0;
	virtual void set_shadow_paper_color( const Color& ) = 0;
	virtual void set_drawing_accent( float_t ) = 0;
	virtual void set_drawing_line_type( int_t ) = 0;

	virtual void set_eye_position( const Vector3& ) = 0;

	virtual GameConstantBuffer* get_game_render_data() const = 0;
	virtual FrameConstantBuffer* get_frame_render_data() const = 0;
	virtual FrameDrawingConstantBuffer* get_frame_drawing_render_data() const = 0;
	virtual ObjectConstantBuffer* get_shared_object_render_data() const = 0;

	const ShaderResource* get_current_object_shader_resource() const { return current_object_shader_resource_; }
	const ShaderResource* get_current_skinning_shader_resource() const { return current_skinning_shader_resource_; }

	virtual const InputLayout* get_input_layout( const char_t* ) const = 0;
	virtual const EffectTechnique* get_effect_technique( const char_t* ) const = 0;

	virtual void set_input_layout( const char_t* ) const = 0;
	virtual void set_input_layout( const InputLayout* ) const = 0;

	virtual void set_primitive_topology( PrimitiveTopology ) const = 0;

	virtual void setup_rendering() const = 0;
	virtual void render_technique( const char_t*, const std::function< void () >& ) const = 0;
	virtual void render_technique( const EffectTechnique*, const std::function< void () >& ) const = 0;
	virtual void render_background() const = 0;
	virtual void render_active_objects( const ActiveObjectManager* ) const;

	/// @todo 整理する
	virtual void set_fade_color( const Color& ) = 0;

	virtual void start_fade_in( float_t speed ) = 0;
	virtual void start_fade_out( float_t speed ) = 0;

	virtual void fade_in( float_t speed = 1.f ) = 0;
	virtual void fade_out( float_t speed = 1.f ) = 0;

	virtual void render_fader() const = 0;

	/// @todo ちゃんと作る Font, size, 指定した Texture への描画
	virtual void draw_text( float_t, float_t, float_t, float_t, const char_t*, const Color& ) const = 0;
	virtual void draw_text_at_center( const char_t*, const Color& ) const = 0;

	void set_debug_axis_enabled( bool e ) { is_debug_axis_enabled_ = e; }

	void render_debug_axis( const ActiveObjectManager* ) const;

	virtual void clear_debug_bullet() const = 0;
	virtual void render_debug_bullet() const = 0;
	
	// void update_frame_render_data( const Camera*, const Vector3& ) const;

}; // class GraphicsManager

} // namespace graphics

} // namespace blue_sky
