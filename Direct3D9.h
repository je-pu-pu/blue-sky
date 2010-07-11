#ifndef DIRECT_3D_9_H
#define DIRECT_3D_9_H

#include <d3dx9.h>
#include <list>

class Direct3D9Resource;
class Direct3D9TextureManager;

/**
 * Direct3D 9 のラッパークラス
 *
 */
class Direct3D9
{
public:
	typedef std::list< Direct3D9Resource* > ResourceList;

private:
	LPDIRECT3D9				direct_3d_;		///< Direct3D 9 
	LPDIRECT3DDEVICE9		device_;		///< Direct3D 9 Device
	LPD3DXEFFECT			effect_;		///< Effect
	LPD3DXSPRITE			sprite_;		///< Sprite

	ResourceList			resource_list_;	///< Resource List
	Direct3D9TextureManager*texture_manager_;

	D3DPRESENT_PARAMETERS	present_;		///< Present Parameters
	D3DCAPS9				device_caps_;	///< Device Caps

	void text_out_adapter_info( const char*, bool = false );
	void text_out_device_caps( const char*, bool = false );

public:
	Direct3D9( HWND, int, int, bool, int = 0, int = 0 );
	~Direct3D9();
	
	void load_effect_file( const char* );

	void reset();

	void set_size( int, int );
	void set_full_screen( bool );
	void set_multi_sample( int, int );

	LPDIRECT3DDEVICE9 getDevice() const { return device_; }

	const D3DPRESENT_PARAMETERS& getPresentParameters() const { return present_; }
	const D3DCAPS9& getCaps() const { return device_caps_; }

	LPD3DXEFFECT getEffect() const { return effect_; }
	LPD3DXSPRITE getSprite() const { return sprite_; }

	Direct3D9TextureManager* getTextureManager() const { return texture_manager_; }

	void add_resource( Direct3D9Resource* r ) { resource_list_.push_back( r ); }

}; // class Direct3D9

#endif // DIRECT_3D_H