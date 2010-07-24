#include "Direct3D9Font.h"
#include "Direct3D9.h"
#include "DirectX.h"

#include <common/exception.h>

Direct3D9Font::Direct3D9Font( Direct3D9* d3d )
	: Direct3D9Resource( d3d )
	, font_( 0 )
{
	// Font
	DIRECT_X_FAIL_CHECK( D3DXCreateFont( direct_3d()->getDevice(), 32, 0, FW_NORMAL, 1, false, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "HuiFontP", & font_ ) );
}

Direct3D9Font::~Direct3D9Font()
{
	if ( font_ )
	{
		font_->Release();
	}
}

void Direct3D9Font::draw_text( int x, int y, const char* text, D3DCOLOR color )
{
	RECT rect = { x, y, direct_3d()->getPresentParameters().BackBufferWidth, direct_3d()->getPresentParameters().BackBufferHeight };
	font_->DrawText( 0, text, -1, & rect, DT_LEFT | DT_NOCLIP, color );
}

void Direct3D9Font::on_lost_device()
{
	if ( font_ )
	{
		DIRECT_X_FAIL_CHECK( font_->OnLostDevice() );
	}
}

void Direct3D9Font::on_reset_device()
{
	if ( font_ )
	{
		DIRECT_X_FAIL_CHECK( font_->OnResetDevice() );
	}
}