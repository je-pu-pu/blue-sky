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
	ObjFileLoader( Model* model, const char_t* name )
		: BaseFileLoader( model, name )
	{ }

	bool load( const char_t* ) override;

}; // class ObjFileLoader

} // namespace blue_sky::graphics
