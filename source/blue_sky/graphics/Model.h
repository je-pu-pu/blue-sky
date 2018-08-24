#pragma once

#include "Mesh.h"
#include "Line.h"
#include <game/Model.h>
#include <vector>

namespace game
{
	class Shader;
}

namespace core
{
	class SkinningAnimationSet;
}

namespace blue_sky::graphics
{
	class Mesh;
	class Line;

/**
 * blue_sky モデルクラス
 *
 */
class Model : public game::Model
{
public:
	using Mesh					= Mesh;
	using Line					= Line;
	using Shader				= game::Shader;
	using ShaderList			= std::vector< Shader* >;
	using SkinningAnimationSet	= core::SkinningAnimationSet;

private:
	Mesh*					mesh_;
	std::unique_ptr< Line >	line_; ///< @todo Mesh にまとめる？
	ShaderList				shader_list_;
	SkinningAnimationSet*	skinning_animation_set_;

	Model& operator = ( const Model& m ) = delete;
public:
	Model()
		: mesh_( 0 )
		, skinning_animation_set_( 0 )
	{ }

	Model( const Model& m )
		: mesh_( m.mesh_ )
		, line_() /// @todo 実装する
		, shader_list_( m.shader_list_ )
		, skinning_animation_set_( m.skinning_animation_set_ )
	{ }

	virtual ~Model() { }

	Mesh* get_mesh() override { return mesh_; };
	const Mesh* get_mesh() const override { return mesh_; };
	void set_mesh( game::Mesh* m ) override { mesh_ = static_cast< Mesh* >( m ); }

	/// @todo Mesh にまとめる
	Line* get_line() { return line_.get(); }
	const Line* get_line() const { return line_.get(); }
	void set_line( Line* l ) { line_.reset( l ); }

	Shader* get_shader_at( uint_t n ) override { return shader_list_[ n ]; }
	const Shader* get_shader_at( uint_t n ) const override { return shader_list_[ n ]; }
	void set_shader_at( uint_t n , Shader* s ) override
	{
		if ( n >= get_shader_count() )
		{
			shader_list_.resize( n + 1, nullptr );
		}
		
		shader_list_[ n ] = s;
	}
	
	uint_t get_shader_count() const override { return shader_list_.size(); }

	SkinningAnimationSet* get_skinning_animation_set() { return skinning_animation_set_; }
	const SkinningAnimationSet* get_skinning_animation_set() const { return skinning_animation_set_; }
	void set_skinning_animation_set( SkinningAnimationSet* sas ) { skinning_animation_set_ = sas; }

	bool is_skin_mesh() const { return skinning_animation_set_; }

	virtual Shader* create_shader() const;
	virtual string_t get_texture_file_path_by_texture_name( const char* ) const;

	void render() const;
};

} // namespace blue_sky::graphics
