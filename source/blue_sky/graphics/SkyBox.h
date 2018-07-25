#pragma once

#include <blue_sky/graphics/Model.h>

namespace blue_sky::graphics
{

/**
 * SkyBox
 *
 */
class SkyBox : public Model
{
public:

private:
	const char* name_;
	const char* ext_;

protected:
	virtual string_t get_texture_file_path_by_texture_name( const char* ) const;

public:
	SkyBox( const char*, const char* = "png" );
	virtual ~SkyBox();

}; // class SkyBox

} // namespace blue_sky::graphics
