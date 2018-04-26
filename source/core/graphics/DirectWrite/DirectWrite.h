#pragma once

#include <core/type.h>
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
	ID2D1Factory*				direct_2d_factory_;
	IDWriteFactory*				dwrite_factory_;
	IDWriteFontCollection*		font_collection_;
	IDWriteTextFormat*			text_format_;
	ID2D1RenderTarget*			render_target_;
	ID2D1SolidColorBrush*		solid_color_brush_;

	void draw_text_common( FLOAT, FLOAT, FLOAT, FLOAT, const WCHAR*, const Color& );

public:
	DirectWrite( IDXGISurface1* );
	~DirectWrite();

	FLOAT get_font_size() const { return 26.f; }
	FLOAT get_font_height() const;

	void begin();
	void end();

	void draw_text( FLOAT, FLOAT, const WCHAR*, const Color& );
	void draw_text( FLOAT, FLOAT, FLOAT, FLOAT, const WCHAR*, const Color& );
	void draw_text_center( FLOAT, FLOAT, FLOAT, FLOAT, const WCHAR*, const Color& );

	FLOAT get_text_height( const WCHAR*, FLOAT, FLOAT ) const;

}; // class DirectWrite
