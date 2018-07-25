#include "SkyBox.h"
#include <blue_sky/graphics/ObjFileLoader.h>

namespace blue_sky::graphics
{

SkyBox::SkyBox( const char* name, const char* ext )
	: name_( name )
	, ext_( ext )
{
	/// @todo ÉçÅ[ÉhÇ∑ÇÈ
	// load_obj( "media/model/sky-box.obj" );
}

SkyBox::~SkyBox()
{

}

string_t SkyBox::get_texture_file_path_by_texture_name( const char* texture_name ) const
{
	return std::string( "media/model/" ) + name_ + "-" + texture_name + "." + ext_;
}

} // namespace blue_sky::graphics
