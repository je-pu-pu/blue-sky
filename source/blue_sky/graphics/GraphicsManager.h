#pragma once

#include <blue_sky/ConstantBuffers.h>
#include <blue_sky/graphics/Model.h>
#include <blue_sky/graphics/Mesh.h>
#include <blue_sky/type.h>

#include <core/animation/SkinningAnimationSet.h>

#include <core/graphics/GraphicsManager.h>
#include <core/graphics/PrimitiveTopology.h>
#include <core/graphics/Mesh.h>
#include <core/graphics/Shader.h>

#include <core/ResourceManager.h>

#include <vector>
#include <functional>


namespace core
{
	class SkinningAnimationSet;

	namespace graphics
	{
		class InputLayout;
		class EffectTechnique;
		class ShadowMap;

		class Shader;
		class Texture;
		class RenderTarget;
		class RenderTargetTexture;
		class BackBufferTexture;
	}

	namespace ecs
	{
		class TransformComponent;
		class CameraComponent;
	}
}

namespace blue_sky
{
	class ActiveObject;
	class ActiveObjectManager;
}

namespace blue_sky::graphics
{
	class Fader;
	class ObjFileLoader;
	class FbxFileLoader;

	namespace shader
	{
		class BaseShadowMapShader;
	}

/**
 * グラフィック管理クラス
 *
 * このクラスが実体化される事はない。
 * 実際にはこのクラスを継承した blue_sky::graphics::direct_3d_11::GraphicsManager が実体化される。
 */
class GraphicsManager : public core::graphics::GraphicsManager
{
public:
	using Model							= Model;
	using Mesh							= Mesh;
	using Line							= Line;
	using Shader						= core::graphics::Shader;
	using Texture						= core::graphics::Texture;
	using RenderTarget					= core::graphics::RenderTarget;
	using RenderTargetTexture			= core::graphics::RenderTargetTexture;
	using BackBufferTexture				= core::graphics::BackBufferTexture;

	using SkinningAnimationSet			= core::SkinningAnimationSet;
	
	using ModelManager					= core::ResourceManager< Model >;
	using MeshManager					= core::ResourceManager< Mesh >;
	using ShaderManager					= core::ResourceManager< Shader >;
	using TextureManager				= core::ResourceManager< Texture >;
	using SkinningAnimationSetManager	= core::ResourceManager< SkinningAnimationSet >;
	
	using PrimitiveTopology				= core::graphics::PrimitiveTopology;
	using InputLayout					= core::graphics::InputLayout;
	using EffectTechnique				= core::graphics::EffectTechnique;
	using ConstantBuffer				= core::graphics::ConstantBuffer;
	using ShadowMap						= core::graphics::ShadowMap;
	using Sprite						= core::graphics::Sprite;

	using TransformComponent			= core::ecs::TransformComponent;
	using CameraComponent				= core::ecs::CameraComponent;
	
	using BaseShadowMapShader			= graphics::shader::BaseShadowMapShader;
	
private:
	std::unique_ptr< Fader >	fader_;
	bool_t						is_fading_in_ = true;		///< true : 現在フェードイン中 or false : 現在フェードアウト中
	float_t						fade_speed_ = 0.f;

	std::unique_ptr< Model >	post_effect_rectangle_;		///< ポストエフェクトレンダリング用矩形

	mutable std::unique_ptr< ShadowMap >					shadow_map_;
	BaseShadowMapShader*									shadow_map_shader_ = nullptr;
	BaseShadowMapShader*									shadow_map_skin_shader_ = nullptr;

	std::vector< Texture* >		paper_texture_list_;
	Texture*					paper_texture_ = nullptr;

	ModelManager				model_manager_;
	MeshManager					mesh_manager_;
	ShaderManager				shader_manager_;
	TextureManager				texture_manager_;
	SkinningAnimationSetManager skinning_animation_set_manager_;

	Texture*					null_texture_ = nullptr;

	mutable const ConstantBuffer* current_object_constant_buffer_ = nullptr;
	mutable const ConstantBuffer* current_skinning_constant_buffer_ = nullptr;

	// Camera
	const TransformComponent*	main_camera_transform_component_ = nullptr;
	const CameraComponent*		main_camera_component_ = nullptr;

	bool						is_debug_axis_enabled_ = true;

	mutable uint_t				pass_count_ = 0;
	mutable uint_t				draw_count_ = 0;

protected:
	const ActiveObjectManager* get_game_object_manager() const;

	bool_t load_mesh( Model*, const char_t* name );
	bool_t load_line( Model*, const char_t* nmae );

	[[nodiscard]] virtual ShadowMap* create_shadow_map( uint_t, uint_t ) const = 0;
	[[nodiscard]] virtual Line* create_line() const = 0;
	[[nodiscard]] virtual Texture* load_texture_file( const char_t* ) const = 0;

	void update_constant_buffers() const;

	void render_shadow_map( const BaseShadowMapShader* , bool ) const;

	void render_debug_axis_for_bones( const ActiveObject* ) const;
	virtual void render_debug_axis_model() const = 0;

public:
	GraphicsManager();
	virtual ~GraphicsManager() override;

	virtual int get_screen_width() const = 0;		///< 画面の幅を返す
	virtual int get_screen_height() const = 0;		///< 画面の高さを返す

	void setup_shadow_map( uint_t, uint_t );
	void unset_shadow_map();

	void update();

	virtual void set_default_viewport() = 0;
	virtual void set_viewport( float_t x, float_t y, float_t w, float_t h, float_t min_d = 0.f, float_t max_d = 1.f ) = 0;

	virtual RenderTargetTexture* create_render_target_texture() = 0;
	virtual BackBufferTexture* get_back_buffer_texture() = 0;
	virtual void set_render_target( RenderTarget* );

	// Model
	template< typename Type=Model > Type* create_named_model( const char_t* name ) { return model_manager_.create_named< Type >( name ); }
	Model* load_model( const char_t* name );
	Model* clone_model( const Model* );
	
	template< typename Type=Model > Type* get_model( const char_t* name ) { return model_manager_.get< Type >( name ); }
	
	// Mesh
	template< typename Type=Mesh > Type* create_named_mesh( const char_t* name, Mesh::Buffer::Type type )
	{
		Mesh::Buffer* b = create_mesh_buffer( type );
		Type* m = mesh_manager_.create_named< Type >( name, b );
		b->set_mesh( m );
		
		return m;
	}

	Mesh* get_mesh( const char_t* name ) { return mesh_manager_.get( name ); }

	virtual Mesh::Buffer* create_mesh_buffer( Mesh::Buffer::Type ) const = 0;

	// Shader
	template< typename Type, typename ... Args > Type* create_shader( Args ... args ) { return shader_manager_.create< Type >( args ... ); }
	template< typename Type, typename ... Args > Type* create_named_shader( const char_t* name, Args ... args ) { return shader_manager_.create_named< Type >( name, args ... ); }
	template< typename Type=Shader > Type* get_shader( const char_t* name ) { return shader_manager_.get< Type >( name ); }
	Shader* clone_shader( const Shader* s ) { Shader* s2 = s->clone(); shader_manager_.add( s2 ); return s2; }

	const ShaderManager::ResourceList& get_shader_list() const { return shader_manager_.get_resource_list(); }

	// SkinningAnimationSet
	SkinningAnimationSet* create_skinning_animation_set() { return skinning_animation_set_manager_.create(); }

	// Texture
	Texture* load_texture( const char_t* file_name ) { return load_named_texture( file_name, file_name ); }
	Texture* load_named_texture( const char_t* name, const char_t* file_name );
	Texture* get_texture( const char_t* name );
	Texture* get_null_texture() { return null_texture_; }

	void setup_default_shaders();
	void load_paper_textures();

	// virtual void unload_mesh( const char_t* ) = 0;
	// virtual void unload_mesh_all() = 0;

	virtual void unload_named_texture( const char_t* );
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

	virtual void set_eye_position( const Vector& ) = 0;

	bool_t is_shadow_enabled() const { return static_cast< bool_t >( shadow_map_ ); }
	ShadowMap* get_shadow_map() { return shadow_map_.get(); }
	const ShadowMap* get_shadow_map() const { return shadow_map_.get(); }

	virtual void resolve_depth_texture() const = 0;
	virtual Texture* get_depth_texture() const = 0;
	virtual Sprite* get_sprite() const = 0;

	virtual GameConstantBuffer* get_game_render_data() const = 0;
	virtual FrameConstantBuffer* get_frame_render_data() const = 0;
	virtual FrameDrawingConstantBuffer* get_frame_drawing_render_data() const = 0;
	virtual ObjectConstantBuffer* get_shared_object_render_data() const = 0;

	void set_current_object_constant_buffer( const ConstantBuffer* r ) const { current_object_constant_buffer_ = r; }
	void set_current_skinning_constant_buffer( const ConstantBuffer* r ) const { current_skinning_constant_buffer_ = r; }
	
	const ConstantBuffer* get_current_object_constant_buffer() const { return current_object_constant_buffer_; }
	const ConstantBuffer* get_current_skinning_constant_buffer() const { return current_skinning_constant_buffer_; }

	virtual const InputLayout* get_input_layout( const char_t* ) const = 0;
	virtual const EffectTechnique* get_effect_technique( const char_t* ) const = 0;

	virtual void set_input_layout( const char_t* ) const = 0;
	virtual void set_input_layout( const InputLayout* ) const = 0;

	virtual void set_primitive_topology( PrimitiveTopology ) const = 0;

	virtual void set_depth_stencil() const = 0;
	virtual void unset_depth_stencil() const = 0;

	virtual void setup_rendering();
	virtual void render_technique( const char_t*, const std::function< void () >& ) const = 0;
	virtual void render_technique( const EffectTechnique*, const std::function< void () >& ) const = 0;
	virtual void render_background() const = 0;
	
	virtual void render_shadow_map() const;
	virtual void render_active_objects( const ActiveObjectManager* ) const;

	virtual void set_post_effect_shader( Shader* );
	virtual void render_post_effect( Texture* );
	virtual void render_post_effect( Texture*, RenderTarget* );

	Fader* get_fader() { return fader_.get(); }
	const Fader* get_fader() const { return fader_.get(); }

	/// @todo 整理する
	void set_fade_color( const Color& );

	void start_fade_in( float_t speed );
	void start_fade_out( float_t speed );

	void fade_in( float_t speed = 1.f );
	void fade_out( float_t speed = 1.f );

	void render_fader() const;

	/// @todo ちゃんと作る Font, size, 指定した Texture への描画
	virtual void draw_text( float_t, float_t, float_t, float_t, const char_t*, const Color& ) const = 0;
	virtual void draw_text_at_center( const char_t*, const Color& ) const = 0;

	void set_debug_axis_enabled( bool e ) { is_debug_axis_enabled_ = e; }

	void render_debug_axis( const ActiveObjectManager* ) const;

	virtual void clear_debug_bullet() const = 0;
	virtual void render_debug_bullet() const = 0;

	/**
	 * メインカメラの情報を設定する
	 * 
	 * @param transform 位置・回転情報
	 * @param camera カメラ情報 ( FOV など )
	 */
	void set_main_camera_info( const TransformComponent* transform, const CameraComponent* camera )
	{
		main_camera_transform_component_ = transform;
		main_camera_component_ = camera;
	}

	void clear_pass_count() const { pass_count_ = 0; }
	void count_pass() const { pass_count_++; }
	uint_t get_pass_count() const { return pass_count_; }

	void clear_draw_count() const { draw_count_ = 0; }
	void count_draw() const { draw_count_++; }
	uint_t get_draw_count() const { return draw_count_; }

	void debug_print_resources() const;

	// void update_frame_render_data( const Camera*, const Vector3& ) const;

}; // class GraphicsManager

} // namespace blue_sky::graphics
