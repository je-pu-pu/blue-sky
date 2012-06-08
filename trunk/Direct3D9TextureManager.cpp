#include "Direct3D9TextureManager.h"
#include "Direct3D9.h"
#include "DirectX.h"

#include <common/exception.h>
#include <common/serialize.h>
#include <common/log.h>

#include <string>

Direct3D9TextureManager::Direct3D9TextureManager( const Direct3D9* direct_3d )
	: direct_3d_( direct_3d )
{

}

Direct3D9TextureManager::~Direct3D9TextureManager()
{
	unload_all();
}

Direct3D9TextureManager::Texture* Direct3D9TextureManager::load( const char* name, const char* file_name )
{
	TextureList::iterator i = texture_list_.find( name );

	if ( i != texture_list_.end() )
	{
		return i->second;
	}

	Texture* texture = 0;
	
	// DIRECT_X_FAIL_CHECK( D3DXCreateTextureFromFile( direct_3d_->getDevice(), file_name, & texture ) );
	DIRECT_X_FAIL_CHECK( D3DXCreateTextureFromFileEx( direct_3d_->getDevice(), file_name, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, 0, NULL, NULL, & texture ) );
	
	texture_list_[ name ] = texture;

	return texture;
}

Direct3D9TextureManager::Texture* Direct3D9TextureManager::get( const char* name ) const
{
	TextureList::const_iterator i = texture_list_.find( name );

	if ( i != texture_list_.end() )
	{
		return i->second;
	}

	return 0;
}

void Direct3D9TextureManager::unload( const char* name )
{
	TextureList::iterator i = texture_list_.find( name );

	if ( i != texture_list_.end() )
	{
		i->second->Release();
		texture_list_.erase( i );
	}
}

void Direct3D9TextureManager::unload_all()
{
	for ( TextureList::iterator i = texture_list_.begin(); i != texture_list_.end(); ++i )
	{
		i->second->Release();
	}

	texture_list_.clear();
}