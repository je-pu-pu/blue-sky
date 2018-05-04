#pragma once

#include "Direct3D11Texture.h"

#include <d3d11.h>

#include <unordered_map>
#include <string>

class Direct3D11;
class Direct3D11Texture;

/**
 * テクスチャ管理
 *
 * テクスチャに名前を付けて管理し、同じテクスチャを重複して読み込まないようにする。
 */
class Direct3D11TextureManager
{
public:
	typedef Direct3D11Texture Texture;
	typedef std::unordered_map< std::string, Texture* > TextureList;

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
