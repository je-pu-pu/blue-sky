#include "TextureReadback.h"
#include "Texture.h"
#include "RenderTargetTexture.h"

#include <wrl/client.h>
#include <wincodec.h>
#include <cstdio>
#include <cstring>

#pragma comment( lib, "windowscodecs.lib" )	// PNG の保存（WIC）

namespace core::graphics::direct_3d_11
{

namespace
{

/// MSAA を解決できない形式（深度・型の無い形式）か
bool is_unresolvable_format( DXGI_FORMAT format )
{
	switch ( format )
	{
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R32G32_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
			return true;
		default:
			return false;
	}
}

} // namespace

ID3D11Texture2D* get_texture_2d( const core::graphics::Texture* texture )
{
	if ( const auto* t = dynamic_cast< const Texture* >( texture ) )
	{
		return t->get_texture_2d();
	}

	if ( const auto* rtt = dynamic_cast< const RenderTargetTexture* >( texture ) )
	{
		return rtt->get_texture() ? rtt->get_texture()->get_texture_2d() : nullptr;
	}

	return nullptr;
}

unsigned int get_bytes_per_pixel( DXGI_FORMAT format )
{
	switch ( format )
	{
		case DXGI_FORMAT_R8_UINT:
		case DXGI_FORMAT_R8_UNORM:
			return 1;

		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_R32_FLOAT:
		case DXGI_FORMAT_R32_UINT:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
			return 4;

		case DXGI_FORMAT_R32G32_TYPELESS:
		case DXGI_FORMAT_R32G32_FLOAT:
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			return 8;

		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			return 16;

		default:
			return 0;
	}
}

bool read_texture_2d(
	ID3D11Device* device,
	ID3D11DeviceContext* context,
	ID3D11Texture2D* source,
	std::vector< std::uint8_t >& out_bytes,
	UINT& out_width,
	UINT& out_height )
{
	using Microsoft::WRL::ComPtr;

	if ( ! device || ! context || ! source )
	{
		return false;
	}

	D3D11_TEXTURE2D_DESC desc{};
	source->GetDesc( & desc );

	const UINT bytes_per_pixel = get_bytes_per_pixel( desc.Format );

	if ( bytes_per_pixel == 0 )
	{
		return false;
	}

	// MSAA なら非 MSAA テクスチャへ解決する（深度は解決できない）
	ComPtr< ID3D11Texture2D > resolved;
	if ( desc.SampleDesc.Count > 1 )
	{
		if ( is_unresolvable_format( desc.Format ) )
		{
			return false;
		}

		D3D11_TEXTURE2D_DESC rd = desc;
		rd.SampleDesc.Count = 1;
		rd.SampleDesc.Quality = 0;
		rd.Usage = D3D11_USAGE_DEFAULT;
		rd.BindFlags = 0;
		rd.CPUAccessFlags = 0;
		rd.MiscFlags = 0;

		if ( FAILED( device->CreateTexture2D( & rd, nullptr, resolved.GetAddressOf() ) ) )
		{
			return false;
		}

		context->ResolveSubresource( resolved.Get(), 0, source, 0, desc.Format );
	}
	else
	{
		resolved = source;
	}

	// CPU から読めるステージングテクスチャへコピー
	D3D11_TEXTURE2D_DESC sd = desc;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Usage = D3D11_USAGE_STAGING;
	sd.BindFlags = 0;
	sd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	sd.MiscFlags = 0;

	ComPtr< ID3D11Texture2D > staging;
	if ( FAILED( device->CreateTexture2D( & sd, nullptr, staging.GetAddressOf() ) ) )
	{
		return false;
	}

	context->CopyResource( staging.Get(), resolved.Get() );

	D3D11_MAPPED_SUBRESOURCE mapped{};
	if ( FAILED( context->Map( staging.Get(), 0, D3D11_MAP_READ, 0, & mapped ) ) )
	{
		return false;
	}

	// 行ピッチを詰めて連続バッファ化する
	const size_t stride = static_cast< size_t >( desc.Width ) * bytes_per_pixel;
	out_bytes.resize( stride * desc.Height );
	const std::uint8_t* src = static_cast< const std::uint8_t* >( mapped.pData );
	for ( UINT y = 0; y < desc.Height; ++y )
	{
		std::memcpy(
			out_bytes.data() + static_cast< size_t >( y ) * stride,
			src + static_cast< size_t >( y ) * mapped.RowPitch,
			stride );
	}
	context->Unmap( staging.Get(), 0 );

	out_width = desc.Width;
	out_height = desc.Height;
	return true;
}

bool read_texture_2d_to_rgba8(
	ID3D11Device* device,
	ID3D11DeviceContext* context,
	ID3D11Texture2D* source,
	std::vector< std::uint8_t >& out_pixels,
	UINT& out_width,
	UINT& out_height )
{
	if ( ! source )
	{
		return false;
	}

	D3D11_TEXTURE2D_DESC desc{};
	source->GetDesc( & desc );

	if ( get_bytes_per_pixel( desc.Format ) != 4 )
	{
		return false;
	}

	return read_texture_2d( device, context, source, out_pixels, out_width, out_height );
}

bool save_texture_2d_to_png(
	ID3D11Device* device,
	ID3D11DeviceContext* context,
	ID3D11Texture2D* source,
	const wchar_t* file_name )
{
	using Microsoft::WRL::ComPtr;

	if ( ! source || ! file_name )
	{
		return false;
	}

	D3D11_TEXTURE2D_DESC desc{};
	source->GetDesc( & desc );

	// バックバッファは BGRA。並びが違うと赤と青が入れ替わる
	const bool bgra =
		desc.Format == DXGI_FORMAT_B8G8R8A8_UNORM || desc.Format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB ||
		desc.Format == DXGI_FORMAT_B8G8R8X8_UNORM || desc.Format == DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;

	std::vector< std::uint8_t > pixels;
	UINT w = 0, h = 0;

	if ( ! read_texture_2d_to_rgba8( device, context, source, pixels, w, h ) )
	{
		return false;
	}

	// 画面の保存では透明度は意味を持たないので不透明にする（そのままだと画像ビューアで透けて見える）
	for ( size_t i = 3; i < pixels.size(); i += 4 )
	{
		pixels[ i ] = 255;
	}

	ComPtr< IWICImagingFactory > factory;
	if ( FAILED( CoCreateInstance( CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( factory.GetAddressOf() ) ) ) )
	{
		return false;
	}

	ComPtr< IWICBitmap > bitmap;
	if ( FAILED( factory->CreateBitmapFromMemory( w, h, bgra ? GUID_WICPixelFormat32bppBGRA : GUID_WICPixelFormat32bppRGBA, w * 4, static_cast< UINT >( pixels.size() ), pixels.data(), bitmap.GetAddressOf() ) ) )
	{
		return false;
	}

	ComPtr< IWICStream > stream;
	if ( FAILED( factory->CreateStream( stream.GetAddressOf() ) ) || FAILED( stream->InitializeFromFilename( file_name, GENERIC_WRITE ) ) )
	{
		return false;
	}

	ComPtr< IWICBitmapEncoder > encoder;
	if ( FAILED( factory->CreateEncoder( GUID_ContainerFormatPng, nullptr, encoder.GetAddressOf() ) ) || FAILED( encoder->Initialize( stream.Get(), WICBitmapEncoderNoCache ) ) )
	{
		return false;
	}

	ComPtr< IWICBitmapFrameEncode > frame;
	if ( FAILED( encoder->CreateNewFrame( frame.GetAddressOf(), nullptr ) ) || FAILED( frame->Initialize( nullptr ) ) || FAILED( frame->SetSize( w, h ) ) )
	{
		return false;
	}

	WICPixelFormatGUID format = GUID_WICPixelFormat32bppBGRA;
	frame->SetPixelFormat( & format );

	return SUCCEEDED( frame->WriteSource( bitmap.Get(), nullptr ) ) && SUCCEEDED( frame->Commit() ) && SUCCEEDED( encoder->Commit() );
}

bool save_texture_2d_to_raw(
	ID3D11Device* device,
	ID3D11DeviceContext* context,
	ID3D11Texture2D* source,
	const wchar_t* file_name )
{
	std::vector< std::uint8_t > bytes;
	UINT w = 0, h = 0;

	if ( ! file_name || ! read_texture_2d( device, context, source, bytes, w, h ) )
	{
		return false;
	}

	FILE* fp = nullptr;
	if ( _wfopen_s( & fp, file_name, L"wb" ) != 0 || ! fp )
	{
		return false;
	}

	std::fwrite( bytes.data(), 1, bytes.size(), fp );
	std::fclose( fp );

	return true;
}

} // namespace core::graphics::direct_3d_11
