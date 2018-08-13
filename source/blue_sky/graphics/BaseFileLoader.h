#pragma once

#include <type/type.h>

namespace game
{
	class Shader;
	class Texture;
}

namespace core
{
	class SkinningAnimationSet;
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
	typedef blue_sky::graphics::Model	Model; ///< @todo ’ŠÛ‰»‚·‚é
	typedef blue_sky::graphics::Mesh	Mesh; ///< @todo ’ŠÛ‰»‚·‚é
	typedef core::SkinningAnimationSet	SkinningAnimationSet;
	typedef game::Shader				Shader;
	typedef game::Texture				Texture;

private:
	Model* model_;
	string_t name_;

protected:
	Model* get_model() { return model_; }
	const Model* get_model() const { return model_; }

	string_t& get_name() { return name_; }
	const string_t& get_name() const { return name_; }

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
