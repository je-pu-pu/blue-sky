#pragma once

#include "Mesh.h"
#include "Line.h"
#include <core/graphics/Model.h>
#include <vector>

namespace core
{
	class SkinningAnimationSet;

	namespace graphics
	{
		class Shader;
	}
}

namespace blue_sky::graphics
{
	class Mesh;
	class Line;
	class GraphicsManager;

/**
 * blue_sky モデルクラス
 *
 */
class Model : public core::graphics::Model
{
public:
	using Mesh					= Mesh;
	using Line					= Line;
	using Shader				= core::graphics::Shader;
	using ShaderList			= std::vector< Shader* >;
	using SkinningAnimationSet	= core::SkinningAnimationSet;

private:
	Mesh*					mesh_;
	std::shared_ptr< Line >	line_; ///< 複製したモデル同士で共有する（線の形は同じ。以前は unique_ptr で、複製すると線が無くなっていた） @todo Mesh にまとめる？
	ShaderList				shader_list_;
	SkinningAnimationSet*	skinning_animation_set_;

	Model& operator = ( const Model& m ) = delete;
public:
	Model()
		: mesh_( nullptr )
		, skinning_animation_set_( nullptr )
	{ }

	Model( const Model& m )
		: mesh_( m.mesh_ )
		, line_( m.line_ )
		, shader_list_( m.shader_list_ )
		, skinning_animation_set_( m.skinning_animation_set_ )
	{ }

	virtual ~Model() override { }

	Mesh* get_mesh() override { return mesh_; };
	const Mesh* get_mesh() const override { return mesh_; };
	void set_mesh( core::graphics::Mesh* m ) override { mesh_ = static_cast< Mesh* >( m ); }

	/// @todo Mesh にまとめる
	Line* get_line() { return line_.get(); }
	const Line* get_line() const { return line_.get(); }
	void set_line( Line* l ) { line_.reset( l ); }

	Shader* get_shader_at( size_t n ) override { return shader_list_[ n ]; }
	const Shader* get_shader_at( size_t n ) const override { return shader_list_[ n ]; }
	void set_shader_at( size_t n , Shader* s ) override
	{
		if ( n >= get_shader_count() )
		{
			shader_list_.resize( n + 1, nullptr );
		}
		
		shader_list_[ n ] = s;
	}
	
	size_t get_shader_count() const override { return shader_list_.size(); }

	SkinningAnimationSet* get_skinning_animation_set() { return skinning_animation_set_; }
	const SkinningAnimationSet* get_skinning_animation_set() const { return skinning_animation_set_; }
	void set_skinning_animation_set( SkinningAnimationSet* sas ) { skinning_animation_set_ = sas; }

	bool is_skin_mesh() const { return skinning_animation_set_; }

	/// このモデル用のシェーダーを作る（作る先の GraphicsManager は呼ぶ側が渡す。以前は core::Service から取っていた）
	virtual Shader* create_shader( GraphicsManager* graphics_manager ) const;
	virtual string_t get_texture_file_path_by_texture_name( const char* ) const;

	/// テクスチャを登録する名前（既定はテクスチャ名そのまま。"front" のような汎用的な名前が他と衝突するモデルは接頭辞を付ける）
	virtual string_t get_texture_resource_name( const char* texture_name ) const { return texture_name; }

	void render() const override;
};

} // namespace blue_sky::graphics
