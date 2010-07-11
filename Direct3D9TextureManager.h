#ifndef DIRECT_3D_9_TEXTURE_MANAGER_H
#define DIRECT_3D_9_TEXTURE_MANAGER_H

#include <d3dx9.h>
#include <map>
#include <string>

class Direct3D9;

/**
 * テクスチャ管理
 *
 */
class Direct3D9TextureManager
{
public:
	typedef IDirect3DTexture9 Texture;
	typedef std::map< std::string, Texture* > TextureList;

private:
	const Direct3D9* direct_3d_;
	TextureList texture_list_;

public:
	Direct3D9TextureManager( const Direct3D9* );
	~Direct3D9TextureManager();

	Texture* load( const char*, const char* );
	Texture* get( const char* ) const;

}; // class Direct3D9TextureManager

#endif // DIRECT_3D_9_TEXTURE_MANAGER_H