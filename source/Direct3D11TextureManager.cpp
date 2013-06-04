#include "Direct3D11TextureManager.h"
#include "Direct3D11.h"
#include "DirectX.h"

#include <common/exception.h>
#include <common/serialize.h>
#include <common/log.h>

#include <string>

Direct3D11TextureManager::Direct3D11TextureManager( Direct3D11* direct_3d )
	: direct_3d_( direct_3d )
{

}

Direct3D11TextureManager::~Direct3D11TextureManager()
{
	unload_all();
}

Direct3D11TextureManager::Texture* Direct3D11TextureManager::load( const char* name, const char* file_name )
{
	TextureList::iterator i = texture_list_.find( name );

	if ( i != texture_list_.end() )
	{
		return i->second;
	}

	Texture* texture = 0;

	if ( FAILED( D3DX11CreateShaderResourceViewFromFile( direct_3d_->getDevice(), file_name, 0, 0, & texture, 0 ) ) )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( std::string( "file open failed. " ) + file_name );
	}
	
	texture_list_[ name ] = texture;

	return texture;
}

Direct3D11TextureManager::Texture* Direct3D11TextureManager::get( const char* name ) const
{
	TextureList::const_iterator i = texture_list_.find( name );

	if ( i != texture_list_.end() )
	{
		return i->second;
	}

	return 0;
}

void Direct3D11TextureManager::unload( const char* name )
{
	TextureList::iterator i = texture_list_.find( name );

	if ( i != texture_list_.end() )
	{
		i->second->Release();
		texture_list_.erase( i );
	}
}

void Direct3D11TextureManager::unload_all()
{
	for ( TextureList::iterator i = texture_list_.begin(); i != texture_list_.end(); ++i )
	{
		i->second->Release();
	}

	texture_list_.clear();
}