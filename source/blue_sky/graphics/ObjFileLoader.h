#pragma once

#include "BaseFileLoader.h"


namespace blue_sky::graphics
{

/**
 * OBJ File Loader
 *
 */
class ObjFileLoader : public BaseFileLoader
{
public:
	typedef blue_sky::graphics::Model Model; ///< @todo ’ŠÛ‰»‚·‚é
	typedef blue_sky::graphics::Mesh Mesh; ///< @todo ’ŠÛ‰»‚·‚é
	typedef game::Shader Shader;
	typedef game::Texture Texture;

protected:
	Texture* load_texture( const char_t* ) const;

public:
	ObjFileLoader( Model* model, const char_t* name )
		: BaseFileLoader( model, name )
	{ }

	bool load( const char_t* ) override;

}; // class FbxFileLoader

} // namespace blue_sky::graphics
