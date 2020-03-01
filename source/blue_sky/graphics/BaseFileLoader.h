#pragma once

#include "Mesh.h"
#include <type/type.h>
#include <vector>
#include <map>

namespace core
{
	class SkinningAnimationSet;

	namespace graphics
	{
		class Shader;
		class Texture;
	}
}

namespace blue_sky::graphics
{
	class Model;
	class Mesh;

/**
 * File Loader Šî’êƒNƒ‰ƒX
 *
 */
class BaseFileLoader
{
public:
	using Model					= blue_sky::graphics::Model;	///< @todo ’ŠÛ‰»‚·‚é
	using Mesh					= blue_sky::graphics::Mesh;		///< @todo ’ŠÛ‰»‚·‚é
	using SkinningAnimationSet	= core::SkinningAnimationSet;
	using Shader				= core::graphics::Shader;
	using Texture				= core::graphics::Texture;

	using VertexList			= std::vector< Mesh::Vertex >;
	using VertexIndexMap		= std::map< Mesh::Vertex, Mesh::Index >;

private:
	Model* model_;
	string_t name_;

protected:
	Model* get_model() { return model_; }
	const Model* get_model() const { return model_; }

	string_t& get_name() { return name_; }
	const string_t& get_name() const { return name_; }

	Shader* get_null_shader() const;

	Mesh* create_mesh() const;
	Shader* create_shader() const;
	SkinningAnimationSet* create_skinning_animation_set() const;
	
	Texture* load_texture( const char_t* ) const;

public:
	BaseFileLoader( Model* model, const char_t* name )
		: model_( model )
		, name_( name )
	{

	}

	virtual bool load( const char_t* file_name ) = 0;

}; // class BaseFileLoader

} // namespace blue_sky::graphics
