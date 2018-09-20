#pragma once

#include <core/graphics/Sprite.h>
#include <d3d11.h>

namespace direct_x_math
{
	class Vector;
	class Matrix;
	class Color;
}

namespace core::graphics::direct_3d_11
{
	template< typename T > class ShaderResource;

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
		static const int SLOT = 13;

		Matrix transform;
	};

	using ConstantBuffer = core::graphics::direct_3d_11::ShaderResource< ConstantBufferData >;

	static const DXGI_FORMAT IndexBufferFormat = DXGI_FORMAT_R16_UINT;

protected:
	Direct3D*		direct_3d_;
	ConstantBuffer*	constant_buffer_;

	Buffer*			vertex_buffer_;
	Buffer*			index_buffer_;

	const InputLayout*			input_layout_;
	const EffectTechnique*		effect_technique_;

	float_t			ortho_offset_;		///< —§‘ÌŽ‹—p

	void create_vertex_buffer();
	void create_index_buffer();

	void draw( const Rect*, const Texture*, const Rect*, const Color* ) override;

public:
	explicit Sprite( Direct3D* );
	~Sprite();

	void set_transform( const Matrix& ) override;
	void set_ortho_offset( float_t ortho_offset ) override { ortho_offset_ = ortho_offset; }

	void begin() override;
	void end() override;

}; // class Sprite

} // namespace core::graphics::direct_3d_11
