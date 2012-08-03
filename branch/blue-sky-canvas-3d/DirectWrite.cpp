#include "DirectWrite.h"
#include "DirectWriteFontCollectionLoader.h"
#include "DirectX.h"

#pragma comment( lib, "d2d1.lib" )
#pragma comment( lib, "dwrite.lib" )

DirectWrite::DirectWrite( IDXGISurface1* surface )
	: direct_2d_factory_( 0 )
	, dwrite_factory_( 0 )
	, font_collection_( 0 )
	, solid_color_brush_( 0 )
	, text_format_( 0 )
	, render_target_( 0 )
{
	// ?
	DIRECT_X_FAIL_CHECK( D2D1CreateFactory( D2D1_FACTORY_TYPE_SINGLE_THREADED, & direct_2d_factory_ ) );
	
	DIRECT_X_FAIL_CHECK( DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof( IDWriteFactory ), reinterpret_cast< IUnknown** >( & dwrite_factory_ ) ) );
	
	DIRECT_X_FAIL_CHECK( dwrite_factory_->RegisterFontCollectionLoader( DirectWriteFontCollectionLoader::GetLoader() ) );
	DIRECT_X_FAIL_CHECK( dwrite_factory_->RegisterFontFileLoader( DirectWriteFontFileLoader::GetLoader() ) );

	const char* font_file_path = "media/font/uzura.ttf";
	const wchar_t* font_family_name = L"uzura_font";

	DIRECT_X_FAIL_CHECK( dwrite_factory_->CreateCustomFontCollection( DirectWriteFontCollectionLoader::GetLoader(), font_file_path, strlen( font_file_path ) + 1, & font_collection_ ) );
	
	// ?
	DIRECT_X_FAIL_CHECK( dwrite_factory_->CreateTextFormat( font_family_name, font_collection_, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 26.f, L"ja-jp", & text_format_ ) );

	// create_render_target()
	{
		D2D1_RENDER_TARGET_PROPERTIES propaties = D2D1::RenderTargetProperties( D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat( DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED ) );
		DIRECT_X_FAIL_CHECK( direct_2d_factory_->CreateDxgiSurfaceRenderTarget( surface, & propaties, & render_target_ ) );
	}
}

DirectWrite::~DirectWrite()
{
	DIRECT_X_RELEASE( text_format_ );
	DIRECT_X_RELEASE( font_collection_ );

	DIRECT_X_RELEASE( solid_color_brush_ );
	DIRECT_X_RELEASE( render_target_ );

	DIRECT_X_FAIL_CHECK( dwrite_factory_->UnregisterFontFileLoader( DirectWriteFontFileLoader::GetLoader() ) );
	DIRECT_X_FAIL_CHECK( dwrite_factory_->UnregisterFontCollectionLoader( DirectWriteFontCollectionLoader::GetLoader() ) );

	DIRECT_X_RELEASE( dwrite_factory_ );
	DIRECT_X_RELEASE( direct_2d_factory_ );
}

void DirectWrite::begin()
{
	render_target_->BeginDraw();
	render_target_->SetTransform( D2D1::IdentityMatrix() );

	// D2D1::ColorF clear_color( 1.f, 1.f, 1.f, 1.f );
	D2D1::ColorF clear_color( 0.f, 0.f, 0.f, 0.f );
	render_target_->Clear( clear_color );
}

void DirectWrite::end()
{
	// !!! fail check
	DIRECT_X_FAIL_CHECK( render_target_->EndDraw() );
}

void DirectWrite::draw_text( FLOAT left, FLOAT top, const WCHAR* text, const Color& color )
{
	draw_text( left, top, FLT_MAX, FLT_MAX, text, color );
}

void DirectWrite::draw_text( FLOAT left, FLOAT top, FLOAT right, FLOAT bottom, const WCHAR* text, const Color& color )
{
	text_format_->SetTextAlignment( DWRITE_TEXT_ALIGNMENT_LEADING );

	draw_text_common( left, top, right, bottom, text, color );
}

void DirectWrite::draw_text_center( FLOAT left, FLOAT top, FLOAT right, FLOAT bottom, const WCHAR* text, const Color& color )
{
	text_format_->SetTextAlignment( DWRITE_TEXT_ALIGNMENT_CENTER );

	draw_text_common( left, top, right, bottom, text, color );
}

void DirectWrite::draw_text_common( FLOAT left, FLOAT top, FLOAT right, FLOAT bottom, const WCHAR* text, const Color& color )
{
	UINT text_length = wcslen( text );

    D2D1_RECT_F rect = D2D1::RectF( left, top, right, bottom );

	DIRECT_X_FAIL_CHECK( render_target_->CreateSolidColorBrush( D2D1::ColorF( color.r(), color.g(), color.b(), color.a() ), & solid_color_brush_ ) );
	
	render_target_->DrawText( text, text_length, text_format_, rect, solid_color_brush_ );

	DIRECT_X_RELEASE( solid_color_brush_ );
}

FLOAT DirectWrite::get_text_height( const WCHAR* text, FLOAT w, FLOAT h ) const
{
	std::wstring ws = text;

	IDWriteTextLayout* text_layout;
	DIRECT_X_FAIL_CHECK( dwrite_factory_->CreateTextLayout( ws.c_str(), ws.length(), text_format_, w, h, & text_layout ) );

	DWRITE_TEXT_METRICS metrics;
	DIRECT_X_FAIL_CHECK( text_layout->GetMetrics( & metrics ) );

	DIRECT_X_RELEASE( text_layout );

	return metrics.height;
}