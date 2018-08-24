#pragma once

#include "Direct3D11Common.h"
#include <core/type.h>

namespace win
{
	class Rect;
	class Point;
}

namespace game
{
	class Texture;
}

namespace direct_x_math
{
	class Vector;
	class Matrix;
	class Color;
}

class Direct3D11;

namespace core::graphics::direct_3d_11
{
	template< typename T > class ShaderResource;

/**
 * Direct3D 11 Sprite
 *
 * @todo êÆóùÇ∑ÇÈ
 */
class Sprite
{
public:
	using Direct3D		= Direct3D11;
	using Texture		= game::Texture;
	
	using InputLayout	= ID3D11InputLayout;
	using Buffer		= ID3D11Buffer;
	
	using Index			= WORD;
	
	using Rect			= win::Rect;
	using Point			= win::Point;

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

	static const Color&	white_;			/// !!!!!

	float_t			ortho_offset_;		///< óßëÃéãóp

	void create_vertex_buffer();
	void create_index_buffer();

	void draw( const Rect*, const Texture*, const Rect*, const Color* );

public:
	explicit Sprite( Direct3D* );
	~Sprite();

	void begin();

	void set_transform( const Matrix& );
	void set_ortho_offset( float_t ortho_offset ) { ortho_offset_ = ortho_offset; }

	void draw( const Point&, const Texture*, const Rect&, const Color& = white_ );
	void draw( const Rect&, const Texture*, const Rect&, const Color& = white_ );
	void draw( const Rect&, const Texture*, const Color& = white_ );

	void draw( const Texture*, const Rect&, const Color& = white_ );
	void draw( const Texture*, const Color& = white_ );

	void end();

}; // class Sprite

} // namespace core::graphics::direct_3d_11
