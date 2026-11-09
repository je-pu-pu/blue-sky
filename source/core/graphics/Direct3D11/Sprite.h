#pragma once

#include <core/graphics/Sprite.h>
#include <d3d11.h>
#include <memory>
#include <vector>

namespace direct_x_math
{
	class Vector;
	class Matrix;
	class Color;
}

namespace core::graphics::direct_3d_11
{
	template< typename, int > class ConstantBufferTyped;

	class Direct3D11;
	class InputLayout;
	class EffectTechnique;

/**
 * Direct3D 11 Sprite
 *
 */
class Sprite : public core::graphics::Sprite
{
public:
	using Direct3D		= Direct3D11;
	
	using Buffer		= ID3D11Buffer;
	
	using Index			= WORD;

	struct Vertex
	{
		Vector3 Position;
		Vector2 TexCoord;
		Color Color;
	};

	struct ConstantBufferData
	{
		Matrix transform;
	};

	using ConstantBuffer = ConstantBufferTyped< ConstantBufferData, 13 >;

	static const DXGI_FORMAT IndexBufferFormat = DXGI_FORMAT_R16_UINT;

	/// 1 回の描画にまとめる四角形の最大数（頂点 4 × 1024 = 4096 で 16 ビットのインデックスに収まる）
	static constexpr size_t MAX_QUADS = 1024;

protected:
	Direct3D*		direct_3d_;
	std::unique_ptr< ConstantBuffer >	constant_buffer_;

	Buffer*			vertex_buffer_;
	Buffer*			index_buffer_;

	const InputLayout*			input_layout_;
	const EffectTechnique*		effect_technique_;
	const EffectTechnique*		color_technique_;		///< テクスチャなし・頂点カラーのみ
	const EffectTechnique*		premultiplied_technique_;	///< 乗算済みアルファのテクスチャ（UI 用）

	/// 溜めている四角形（同じテクスチャ・同じ描き方の間）
	std::vector< Vertex >		pending_vertices_;
	const EffectTechnique*		pending_technique_ = nullptr;
	const Texture*				pending_texture_ = nullptr;


	void create_vertex_buffer();
	void create_index_buffer();

	void draw( const Rect*, const Texture*, const Rect*, const Color* ) override;

	void draw_uv_impl( const Rect& dst, const Texture* texture, const UVRect& uv, const Color& color, AlphaMode mode ) override;

	void draw_quad( const Rect& dst, const UVRect& uv, const Color& color, const EffectTechnique* technique, const Texture* texture );

public:
	explicit Sprite( Direct3D* );
	~Sprite();

	void set_transform( const Matrix& ) override;
	/// 立体視のずらし（文字の描画と共有するため Direct3D11 に持たせる）
	void set_ortho_offset( float_t ortho_offset ) override;

private:
	/// 頂点バッファ・インデックスバッファ・トポロジー・入力レイアウトを結ぶ
	void bind_input_assembler();

public:

	void begin() override;
	void draw_color( const Rect& dr, const Color& c ) override;
	void end() override;

	/// 溜めている四角形を 1 回で描く
	void flush() override;

}; // class Sprite

} // namespace core::graphics::direct_3d_11
