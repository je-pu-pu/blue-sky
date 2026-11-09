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
	ObjFileLoader( GraphicsManager* graphics_manager, Model* model, const char_t* name )
		: BaseFileLoader( graphics_manager, model, name )
	{ }

	bool load( const char_t* ) override;

}; // class ObjFileLoader

} // namespace blue_sky::graphics
