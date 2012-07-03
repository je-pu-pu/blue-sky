#include "DirectWrite.h"
#include "DirectX.h"

#pragma comment( lib, "d2d1.lib" )
#pragma comment( lib, "dwrite.lib" )

DirectWrite::DirectWrite( IDXGISurface1* surface )
	: direct_2d_factory_( 0 )
	, render_target_( 0 )
	, solid_color_brush_( 0 )
	, dwrite_factory_( 0 )
	, text_format_( 0 )
{
	// ?
	DIRECT_X_FAIL_CHECK( D2D1CreateFactory( D2D1_FACTORY_TYPE_SINGLE_THREADED, & direct_2d_factory_ ) );
	
	DIRECT_X_FAIL_CHECK( DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof( IDWriteFactory ), reinterpret_cast< IUnknown** >( & dwrite_factory_ ) ) );
	DIRECT_X_FAIL_CHECK( dwrite_factory_->CreateTextFormat( L"Gabriola", 0, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 32.f, L"en-us", & text_format_ ) );


	/*
	if ( false )
	{
		RECT rc;
		GetClientRect( hwnd, &rc );

		D2D1_SIZE_U size = D2D1::SizeU( rc.right - rc.left, rc.bottom - rc.top );

		DIRECT_X_FAIL_CHECK( direct_2d_factory_->CreateHwndRenderTarget( D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties( hwnd, size ), & render_target_ ) );
	}
	*/
	{
		FLOAT dpi_x, dpi_y;
		direct_2d_factory_->GetDesktopDpi( & dpi_x, & dpi_y );

		D2D1_RENDER_TARGET_PROPERTIES propaties = D2D1::RenderTargetProperties( D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat( DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED ), dpi_x, dpi_y );
		DIRECT_X_FAIL_CHECK( direct_2d_factory_->CreateDxgiSurfaceRenderTarget( surface, & propaties, & render_target_ ) );
	}

	DIRECT_X_FAIL_CHECK( render_target_->CreateSolidColorBrush( D2D1::ColorF( D2D1::ColorF::Black ), & solid_color_brush_ ) );
}

DirectWrite::~DirectWrite()
{
	DIRECT_X_RELEASE( text_format_ );
	DIRECT_X_RELEASE( dwrite_factory_ );

	DIRECT_X_RELEASE( solid_color_brush_ );
	DIRECT_X_RELEASE( render_target_ );
	DIRECT_X_RELEASE( direct_2d_factory_ );
}

void DirectWrite::begin()
{
	render_target_->BeginDraw();
	render_target_->SetTransform( D2D1::IdentityMatrix() );

	D2D1::ColorF clear_color( 0.f, 0.f, 0.f, 0.f );
	render_target_->Clear( clear_color );
}

void DirectWrite::end()
{
	// !!! fail check
	DIRECT_X_FAIL_CHECK( render_target_->EndDraw() );
}

void DirectWrite::drawText( FLOAT left, FLOAT top, const WCHAR* text )
{
	drawText( left, top, FLT_MAX, FLT_MAX, text );
}

void DirectWrite::drawText( FLOAT left, FLOAT top, FLOAT right, FLOAT bottom, const WCHAR* text )
{
	{
		// render_target_->DrawLine( D2D1::Point2F( 10.f, 10.f ), D2D1::Point2F( 300.f, 300.f ), solid_color_brush_, 10.f );
	}

    UINT text_length = wcslen( text );

    D2D1_RECT_F rect = D2D1::RectF( left, top, right, bottom ); // !!!

	render_target_->DrawText( text, text_length, text_format_, rect, solid_color_brush_ );
}