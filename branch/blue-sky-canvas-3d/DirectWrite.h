#ifndef DIRECT_WRITE_H
#define DIRECT_WRITE_H

#include <dwrite.h>
#include <d2d1.h>

/**
 * DirectWrite のラッパークラス
 *
 */
class DirectWrite
{
public:

private:
	// ?
	ID2D1Factory*				direct_2d_factory_;
//	ID2D1HwndRenderTarget*		render_target_;
	ID2D1RenderTarget*			render_target_;
    ID2D1SolidColorBrush*		solid_color_brush_;

	IDWriteFactory*				dwrite_factory_;
	IDWriteTextFormat*			text_format_;

public:
	DirectWrite( IDXGISurface1* );
	~DirectWrite();

	void begin();
	void end();

	void drawText( FLOAT, FLOAT, const WCHAR* );
	void drawText( FLOAT, FLOAT, FLOAT, FLOAT, const WCHAR* );

}; // class DirectWrite

#endif // DIRECT_WRITE_H