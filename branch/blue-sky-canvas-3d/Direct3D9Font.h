#ifndef DIRECT_3D_9_FONT_H
#define DIRECT_3D_9_FONT_H

#include "Direct3D9Resource.h"
#include <d3dx9.h>

class Direct3D9;

/**
 * ID3DXFont のラッパークラス
 *
 */
class Direct3D9Font : public Direct3D9Resource
{
protected:
	LPD3DXFONT font_;
	
public:
	Direct3D9Font( Direct3D9* );
	~Direct3D9Font();
	
	void load( const char*, const char* );

	void draw_text( int, int, const char*, D3DCOLOR ) const;
	void draw_text_center( int, int, const char*, D3DCOLOR ) const;

	int get_text_height( const char* ) const;

	void on_lost_device();
	void on_reset_device();

}; // class Direct3D9Font

#endif // DIRECT_3D_9_FONT_H
