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
class Direct3D11Texture;

template< typename T > class Direct3D11ConstantBuffer;

/**
 * Direct3D 11 Sprite
 *
 */
class Direct3D11Sprite
{
public:
	typedef Direct3D11					Direct3D;
	typedef game::Texture				Texture;

	typedef Direct3D11Sprite			Sprite;
	
	typedef ID3D11InputLayout			InputLayout;
	typedef ID3D11Buffer				Buffer;

	typedef DirectX::XMFLOAT2			Vector2;
	typedef DirectX::XMFLOAT3			Vector3;
	typedef DirectX::XMFLOAT4			Vector4;
	
	typedef direct_x_math::Color		Color;
	typedef direct_x_math::Vector		Vector;
	typedef direct_x_math::Matrix		Matrix;

	typedef WORD						Index;

	typedef win::Rect					Rect;
	typedef win::Point					Point;

	struct Vertex
	{
		Vector3 Position;
		Vector2 TexCoord;
		Color Color;
	};

	struct ConstantBufferData
	{
		static const int DEFAULT_SLOT = 13;

		Matrix transform;
	};

	typedef Direct3D11ConstantBuffer< ConstantBufferData > ConstantBuffer;

	static const DXGI_FORMAT IndexBufferFormat = DXGI_FORMAT_R16_UINT;

protected:
	Direct3D*		direct_3d_;
	ConstantBuffer*	constant_buffer_;

	Buffer*			vertex_buffer_;
	Buffer*			index_buffer_;

	static Color	white_;				/// !!!!!

	float_t			ortho_offset_;		///< —§‘ÌŽ‹—p

	void create_vertex_buffer();
	void create_index_buffer();

	void draw( const Rect*, const Texture*, const Rect*, const Color* );

public:
	Direct3D11Sprite( Direct3D* );
	~Direct3D11Sprite();

	void begin();

	void set_transform( const Matrix& );
	void set_ortho_offset( float_t ortho_offset ) { ortho_offset_ = ortho_offset; }

	void draw( const Point&, const Texture*, const Rect&, const Color& = white_ );
	void draw( const Rect&, const Texture*, const Rect&, const Color& = white_ );
	void draw( const Rect&, const Texture*, const Color& = white_ );

	void draw( const Texture*, const Rect&, const Color& = white_ );
	void draw( const Texture*, const Color& = white_ );

	void end();

}; // class Direct3D11Sprite
