#include "TextureReadback.h"

#include <wrl/client.h>
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

} // namespace core::graphics::direct_3d_11
