#include "Direct3D11TextureManager.h"
#include "Direct3D11.h"
#include "DirectX.h"

#include "WICTextureLoader.h"

#include <common/string.h>
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

	// common::log( "log/debug.log", string_t( "loading texture : " ) + name + " : " + file_name );

	/*
	D3DX11_IMAGE_LOAD_INFO image_load_info;
	image_load_info.Width = D3DX11_DEFAULT; // 256;
	image_load_info.Height = D3DX11_DEFAULT; // 256;
	image_load_info.Depth = D3DX11_DEFAULT;
	image_load_info.FirstMipLevel = D3DX11_DEFAULT;
	image_load_info.MipLevels = D3DX11_DEFAULT;
	image_load_info.Usage = static_cast< D3D11_USAGE >( D3DX11_DEFAULT );
	image_load_info.BindFlags = D3DX11_DEFAULT;
	image_load_info.CpuAccessFlags = D3DX11_DEFAULT;
	image_load_info.MiscFlags = D3DX11_DEFAULT;
	image_load_info.Format = DXGI_FORMAT_FROM_FILE;
	image_load_info.Filter = D3DX11_DEFAULT;
	image_load_info.MipFilter = D3DX11_DEFAULT;
	image_load_info.pSrcInfo = nullptr;
	*/

	ID3D11ShaderResourceView* view = 0;

	std::wstring ws_file_name = common::convert_to_wstring( file_name );

	if ( FAILED( DirectX::CreateWICTextureFromFile( direct_3d_->getDevice(), ws_file_name.c_str(), nullptr, & view ) ) )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( std::string( "file open failed. " ) + file_name );
	}

	/*
	if ( FAILED( D3DX11CreateShaderResourceViewFromFile( direct_3d_->getDevice(), file_name, & image_load_info, 0, & view, 0 ) ) )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( std::string( "file open failed. " ) + file_name );
	}
	*/
	
	Texture* texture = new Texture( direct_3d_, view );
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
		delete i->second;
		texture_list_.erase( i );
	}
}

void Direct3D11TextureManager::unload_all()
{
	for ( TextureList::iterator i = texture_list_.begin(); i != texture_list_.end(); ++i )
	{
		delete i->second;
	}

	texture_list_.clear();
}