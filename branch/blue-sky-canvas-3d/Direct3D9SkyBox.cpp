#include "Direct3D9SkyBox.h"
#include "Direct3D9.h"
#include "DirectX.h"

#include <common/exception.h>
#include <common/math.h>

Direct3D9SkyBox::Direct3D9SkyBox( Direct3D9* direct_3d, const char* name, const char* ext )
	: Direct3D9Mesh( direct_3d )
	, name_( name )
	, ext_( ext )
{
	load_x( "media/model/sky-box.x" );
}

Direct3D9SkyBox::~Direct3D9SkyBox()
{

}

std::string Direct3D9SkyBox::get_texture_file_name_by_texture_name( const char* texture_name ) const
{
	return std::string( "media/model/" ) + name_ + "-" + texture_name + "." + ext_;
}

void Direct3D9SkyBox::render()
{
	for ( unsigned int n = 0; n < material_count_; n++ )
	{
		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetMaterial( & materials_[ n ] ) );
		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->SetTexture( 0, textures_[ n ] ) );

		DIRECT_X_FAIL_CHECK( mesh_->DrawSubset( n ) );
	}
}