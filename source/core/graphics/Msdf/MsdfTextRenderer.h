#pragma once

#include "MsdfFont.h"
#include "GlyphCache.h"
#include "GlyphAtlas.h"
#include <core/DirectX.h>
#include <core/type.h>
#include <d3d11.h>
#include <memory>
#include <vector>
#include <string>

namespace core::graphics::direct_3d_11
{
	class Direct3D11;
	class InputLayout;
	class EffectTechnique;
	template< typename, int > class ConstantBufferTyped;
}

namespace core::graphics {

struct TextStyle {
	Color text_color = Color::White;
	Color outline_color = Color::Black;
	float outline_width = 0.f;
	float font_size = 32.f;
};

class MsdfTextRenderer {
public:
	struct Vertex {
		Vector3 position;
		Vector2 tex_coord;
		Color color;
	};

	struct MsdfConstantBufferData {
		Color text_color;
		Color outline_color;
		float outline_width = 0.f;
		float smoothing = 0.f;
		float padding_0 = 0.f;
		float padding_1 = 0.f;
	};

	struct TransformConstantBufferData {
		Matrix transform;
	};

	using MsdfConstantBuffer = direct_3d_11::ConstantBufferTyped< MsdfConstantBufferData, 3 >;
	using TransformConstantBuffer = direct_3d_11::ConstantBufferTyped< TransformConstantBufferData, 13 >;

private:
	direct_3d_11::Direct3D11* direct_3d_;

	std::unique_ptr< MsdfFont > font_;
	std::unique_ptr< GlyphAtlas > atlas_;
	std::unique_ptr< GlyphCache > cache_;

	std::unique_ptr< MsdfConstantBuffer > msdf_constant_buffer_;
	std::unique_ptr< TransformConstantBuffer > transform_constant_buffer_;

	ID3D11Buffer* vertex_buffer_;
	ID3D11Buffer* index_buffer_;

	const direct_3d_11::InputLayout* input_layout_;
	const direct_3d_11::EffectTechnique* effect_technique_;

	std::vector< Vertex > vertices_;
	std::vector< uint16_t > indices_;

	static constexpr int MAX_CHARS = 1024;
	static constexpr int ATLAS_SIZE = 2048;
	static constexpr int CELL_SIZE = 48;

	void create_vertex_buffer();
	void create_index_buffer();
	void update_transform();

	void append_quad( float x, float y, float w, float h, const UVRect& uv, const Color& color );

public:
	MsdfTextRenderer( direct_3d_11::Direct3D11* direct_3d, const char* font_path );
	~MsdfTextRenderer();

	void draw_text( float x, float y, const wchar_t* text, const TextStyle& style );
	void draw_text( float x, float y, const char* text, const TextStyle& style );

	void flush();
};

} // namespace core::graphics
