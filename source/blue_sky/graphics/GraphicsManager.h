#pragma once

#include <SkinningAnimationSet.h>
#include <blue_sky/ShaderResources.h>
#include <blue_sky/graphics/Model.h>
#include <blue_sky/graphics/Mesh.h>

#include <core/graphics/PrimitiveTopology.h>
#include <core/type.h>

#include <game/GraphicsManager.h>
#include <game/ResourceManager.h>
#include <game/Mesh.h>
#include <game/Shader.h>

#include <common/safe_ptr.h>
#include <common/auto_ptr.h>

#include <vector>
#include <functional>

class SkinningAnimationSet;

namespace game
{
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
	class ActiveObject;
	class ActiveObjectManager;

namespace graphics
{
	class Fader;
	class ObjFileLoader;
	class FbxFileLoader;

/**
 * グラフィック管理クラス
 *
 */
class GraphicsManager : public game::GraphicsManager
{
public:
	typedef Model			Model;
	typedef Mesh			Mesh;
	typedef Line			Line;
	typedef game::Shader	Shader;
	typedef game::Texture	Texture;

	typedef game::ResourceManager< Model >					ModelManager;
	typedef game::ResourceManager< Mesh >					MeshManager;
	typedef game::ResourceManager< Shader >					ShaderManager;
	typedef game::ResourceManager< Texture >				TextureManager;
	typedef game::ResourceManager< SkinningAnimationSet >	SkinningAnimationSetManager;

	typedef core::graphics::PrimitiveTopology				PrimitiveTopology;
	typedef core::graphics::InputLayout						InputLayout;
	typedef core::graphics::EffectTechnique					EffectTechnique;

private:
	std::unique_ptr< Fader >	fader_;

	std::vector< Texture* >		paper_texture_list_;
	Texture*					paper_texture_ = 0;

	ModelManager				model_manager_;
	MeshManager					mesh_manager_;
	ShaderManager				shader_manager_;
	TextureManager				texture_manager_;
	SkinningAnimationSetManager skinning_animation_set_manager_;

	mutable const ShaderResource* current_object_shader_resource_ = 0;
	mutable const ShaderResource* current_skinning_shader_resource_ = 0;

	bool is_debug_axis_enabled_ = true;

protected:
	bool_t load_mesh( Model*, const char_t* name ) const;
	bool_t load_line( Model*, const char_t* nmae ) const;

	[[nodiscard]] virtual Line* create_line() const = 0;
	[[nodiscard]] virtual Texture* load_texture_file( const char_t* ) const = 0;

	void render_debug_axis_for_bones( const ActiveObject* ) const;
	virtual void render_debug_axis_model() const = 0;

public:
	GraphicsManager();
	virtual ~GraphicsManager();

	virtual void update() = 0;

	
	Model* create_named_model( const char_t* name ) { return model_manager_.create_named( name ); }
	Model* load_model( const char_t* name );
	Model* clone_model( const Model* );

	// virtual Model* load_model( const char_t* name ) { Model* m = model_manager_.get( name ); return ( m ? m : nullptr ); }

	// virtual Mesh* load_mesh( const char_t*, const char_t* ) = 0;

	// DrawingMesh* load_drawing_mesh( const char_t*, common::safe_ptr< SkinningAnimationSet >& );
	// DrawingLine* load_drawing_line( const char_t* );
	
	// template< typename Type=Model > Model* get_model( const char_t* name ) { return model_manager_.get< Type >( name ); }

	template< typename Type=Mesh > Type* create_named_mesh( const char_t* name )
	{
		Mesh::Buffer* b = create_mesh_buffer();
		Type* m = mesh_manager_.create_named< Type >( name, b );
		b->set_mesh( m );
		
		return m;
	}

	Mesh* get_mesh( const char_t* name ) { return mesh_manager_.get( name ); }

	virtual Mesh::Buffer* create_mesh_buffer() const = 0;

	template< typename Type, typename ... Args > Type* create_shader( Args ... args ) { return shader_manager_.create< Type >( args ... ); }
	template< typename Type, typename ... Args > Type* create_named_shader( const char_t* name, Args ... args ) { return shader_manager_.create_named< Type >( name, args ... ); }
	template< typename Type=Shader > Type* get_shader( const char_t* name ) { return shader_manager_.get< Type >( name ); }
	Shader* clone_shader( const Shader* s ) { Shader* s2 = s->clone(); shader_manager_.add( s2 ); return s2; }

	SkinningAnimationSet* create_skinning_animation_set() { return skinning_animation_set_manager_.create(); }

	Texture* load_texture( const char_t* name, const char_t* file_name );
	Texture* get_texture( const char_t* name );

	void setup_default_shaders();
	void load_paper_textures();

	// virtual void unload_mesh( const char_t* ) = 0;
	// virtual void unload_mesh_all() = 0;

	// virtual void unload_texture( const char_t* ) = 0;
	// virtual void unload_texture_all() = 0;

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

	void set_current_object_shader_resource( const ShaderResource* r ) const { current_object_shader_resource_ = r; }
	void get_current_skinning_shader_resource( const ShaderResource* r ) const { current_skinning_shader_resource_ = r; }
	
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

	Fader* get_fader() { return fader_.get(); }

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
