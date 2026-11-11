#include "TextureReadback.h"

#include <wrl/client.h>
#include <wincodec.h>
#include <cstring>

namespace core::graphics::direct_3d_11
{

bool read_texture_2d_to_rgba8(
	ID3D11Device* device,
	ID3D11DeviceContext* context,
	ID3D11Texture2D* source,
	std::vector< std::uint8_t >& out_pixels,
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

	// MSAA なら非 MSAA テクスチャへ解決する
	ComPtr< ID3D11Texture2D > resolved;
	if ( desc.SampleDesc.Count > 1 )
	{
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
	const UINT stride = desc.Width * 4;
	out_pixels.resize( static_cast< size_t >( stride ) * desc.Height );
	const std::uint8_t* src = static_cast< const std::uint8_t* >( mapped.pData );
	for ( UINT y = 0; y < desc.Height; ++y )
	{
		std::memcpy(
			out_pixels.data() + static_cast< size_t >( y ) * stride,
			src + static_cast< size_t >( y ) * mapped.RowPitch,
			stride );
	}
	context->Unmap( staging.Get(), 0 );

	out_width = desc.Width;
	out_height = desc.Height;
	return true;
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

} // namespace core::graphics::direct_3d_11
