#ifndef DIRECT_3D_11_TEXTURE_MANAGER_H
#define DIRECT_3D_11_TEXTURE_MANAGER_H

#include <d3d11.h>

#include <string>
#include <map>

class Direct3D11;

/**
 * テクスチャ管理
 *
 */
class Direct3D11TextureManager
{
public:
	typedef ID3D11ShaderResourceView Texture;
	typedef std::map< std::string, Texture* > TextureList;

private:
	Direct3D11* direct_3d_;
	TextureList texture_list_;

public:
	Direct3D11TextureManager( Direct3D11* );
	~Direct3D11TextureManager();

	Texture* load( const char*, const char* );
	Texture* get( const char* ) const;

	void unload( const char* );
	void unload_all();

}; // class Direct3D11TextureManager

#endif // DIRECT_3D_11_TEXTURE_MANAGER_H