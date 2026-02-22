#include "GlyphAtlas.h"
#include <algorithm>

namespace core::graphics {

GlyphAtlas::GlyphAtlas( ID3D11Device* device, ID3D11DeviceContext* context, int atlas_size, int cell_size )
	: device_( device )
	, context_( context )
	, atlas_size_( atlas_size )
	, cell_size_( cell_size )
	, cells_per_row_( atlas_size / cell_size )
	, total_cells_( cells_per_row_ * cells_per_row_ )
	, next_cell_index_( 0 )
{
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = atlas_size_;
	desc.Height = atlas_size_;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;

	DIRECT_X_FAIL_CHECK( device_->CreateTexture2D( &desc, nullptr, &texture_ ) );

	D3D11_SHADER_RESOURCE_VIEW_DESC view_desc{};
	view_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	view_desc.Texture2D.MipLevels = 1;

	DIRECT_X_FAIL_CHECK( device_->CreateShaderResourceView( texture_.get(), &view_desc, &view_ ) );
}

static std::vector< uint8_t > convert_msdf_to_rgba( const msdfgen::Bitmap< float, 3 >& bitmap )
{
	int w = bitmap.width();
	int h = bitmap.height();
	std::vector< uint8_t > rgba( w * h * 4 );

	for ( int y = 0; y < h; ++y )
	{
		for ( int x = 0; x < w; ++x )
		{
			const float* pixel = bitmap( x, y );

			// msdfgen のビットマップは下から上 (y=0が下) なので上下反転する
			int dst_y = h - 1 - y;
			int dst = ( dst_y * w + x ) * 4;

			rgba[ dst + 0 ] = static_cast< uint8_t >( std::clamp( pixel[ 0 ] * 255.f + 0.5f, 0.f, 255.f ) );
			rgba[ dst + 1 ] = static_cast< uint8_t >( std::clamp( pixel[ 1 ] * 255.f + 0.5f, 0.f, 255.f ) );
			rgba[ dst + 2 ] = static_cast< uint8_t >( std::clamp( pixel[ 2 ] * 255.f + 0.5f, 0.f, 255.f ) );
			rgba[ dst + 3 ] = 255;
		}
	}

	return rgba;
}

void GlyphAtlas::upload_cell( int cell_index, const std::vector< uint8_t >& rgba_data, int width, int height )
{
	int col = cell_index % cells_per_row_;
	int row = cell_index / cells_per_row_;

	D3D11_BOX box{};
	box.left = col * cell_size_;
	box.top = row * cell_size_;
	box.right = box.left + width;
	box.bottom = box.top + height;
	box.front = 0;
	box.back = 1;

	context_->UpdateSubresource( texture_.get(), 0, &box, rgba_data.data(), width * 4, 0 );
}

int GlyphAtlas::add_glyph( const msdfgen::Bitmap< float, 3 >& bitmap, UVRect& uv )
{
	if ( is_full() )
	{
		return -1;
	}

	int cell_index = next_cell_index_++;

	replace_glyph( cell_index, bitmap, uv );

	return cell_index;
}

void GlyphAtlas::replace_glyph( int cell_index, const msdfgen::Bitmap< float, 3 >& bitmap, UVRect& uv )
{
	auto rgba = convert_msdf_to_rgba( bitmap );

	int w = bitmap.width();
	int h = bitmap.height();

	upload_cell( cell_index, rgba, w, h );

	int col = cell_index % cells_per_row_;
	int row = cell_index / cells_per_row_;

	float inv = 1.f / static_cast< float >( atlas_size_ );

	uv.u0 = col * cell_size_ * inv;
	uv.v0 = row * cell_size_ * inv;
	uv.u1 = ( col * cell_size_ + w ) * inv;
	uv.v1 = ( row * cell_size_ + h ) * inv;
}

} // namespace core::graphics
