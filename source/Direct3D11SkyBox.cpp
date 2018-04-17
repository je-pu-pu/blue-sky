#include "Direct3D11SkyBox.h"
#include "Direct3D11MeshManager.h"
#include "Direct3D11.h"
#include "DirectX.h"

#include <common/exception.h>
#include <common/math.h>

#include <string>

Direct3D11SkyBox::Direct3D11SkyBox( Direct3D* direct_3d, const char* name, const char* ext )
	: Direct3D11Mesh( direct_3d )
	, name_( name )
	, ext_( ext )
{
	if ( std::string( ext_ ).empty() )
	{
		ext_ = "png";
	}

	load_obj( "media/model/sky-box.obj" );
}

Direct3D11SkyBox::~Direct3D11SkyBox()
{

}

string_t Direct3D11SkyBox::get_texture_file_name_by_texture_name( const char* texture_name ) const
{
	return std::string( "media/model/" ) + name_ + "-" + texture_name + "." + ext_;
}
