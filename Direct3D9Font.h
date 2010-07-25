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
	
	void draw_text( int, int, const char*, D3DCOLOR );
	void draw_text_center( int, int, const char*, D3DCOLOR );


	int get_text_height( const char* );

	void on_lost_device();
	void on_reset_device();

}; // class Direct3D9Font

#endif // DIRECT_3D_9_FONT_H
