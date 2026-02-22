#pragma once

#include <core/DirectX.h>
#include <d3d11.h>
#include <vector>
#include <cstdint>

#define MSDFGEN_PUBLIC
#define MSDFGEN_EXT_PUBLIC
#include <msdfgen/core/Bitmap.h>

namespace core::graphics {

struct UVRect {
	float u0, v0;
	float u1, v1;
};

class GlyphAtlas {
	com_ptr< ID3D11Texture2D > texture_;
	com_ptr< ID3D11ShaderResourceView > view_;

	ID3D11Device* device_;
	ID3D11DeviceContext* context_;

	int atlas_size_;
	int cell_size_;
	int cells_per_row_;
	int total_cells_;
	int next_cell_index_;

	void upload_cell( int cell_index, const std::vector< uint8_t >& rgba_data, int width, int height );

public:
	GlyphAtlas( ID3D11Device* device, ID3D11DeviceContext* context, int atlas_size, int cell_size );

	int add_glyph( const msdfgen::Bitmap< float, 3 >& bitmap, UVRect& uv );
	void replace_glyph( int cell_index, const msdfgen::Bitmap< float, 3 >& bitmap, UVRect& uv );

	bool is_full() const { return next_cell_index_ >= total_cells_; }
	int get_cell_size() const { return cell_size_; }
	int get_atlas_size() const { return atlas_size_; }

	ID3D11ShaderResourceView* get_view() const { return view_.get(); }
};

} // namespace core::graphics
