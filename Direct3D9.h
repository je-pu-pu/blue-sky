#ifndef DIRECT_3D_9_H
#define DIRECT_3D_9_H

#include <d3dx9.h>

/**
 * Direct3D 9 のラッパークラス
 *
 */
class Direct3D9
{
private:
	LPDIRECT3D9				direct_3d_;		///< Direct3D 9 
	LPDIRECT3DDEVICE9		device_;		///< Direct3D 9 Device
	LPD3DXEFFECT			effect_;		///< Effect

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
	LPD3DXEFFECT getEffect() const { return effect_; }

}; // class Direct3D9

#endif // DIRECT_3D_H
