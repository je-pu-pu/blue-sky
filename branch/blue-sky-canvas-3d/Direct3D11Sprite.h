#ifndef DIRECT_3D_11_SPRITE_H
#define DIRECT_3D_11_SPRITE_H

#include "Direct3D11Matrix.h"

#include <d3d11.h>
#include <xnamath.h>

namespace win
{
	class Rect;
}

class Direct3D11;
class Direct3D11ConstantBuffer;
class Direct3D11Vector;
class Direct3D11Matrix;
class Direct3D11Color;

/**
 * Direct3D 11 Sprite
 *
 */
class Direct3D11Sprite
{
public:
	typedef Direct3D11					Direct3D;
	typedef Direct3D11ConstantBuffer	ConstantBuffer;
	typedef Direct3D11Color				Color;


	typedef Direct3D11Sprite			Sprite;
	
	typedef ID3D11InputLayout			InputLayout;
	typedef ID3D11Buffer				Buffer;
	typedef ID3D11ShaderResourceView	Texture;

	typedef XMFLOAT2					Vector2;
	typedef XMFLOAT3					Vector3;
	typedef XMFLOAT4					Vector4;
	
	typedef Direct3D11Vector			Vector;
	typedef Direct3D11Matrix			Matrix;

	typedef WORD						Index;

	typedef win::Rect					Rect;

	struct Vertex
	{
		Vector3 Position;
		Vector2 TexCoord;
		Vector4 Color;
	};

	static const DXGI_FORMAT IndexBufferFormat = DXGI_FORMAT_R16_UINT;

protected:
	Direct3D*		direct_3d_;
	ConstantBuffer*	constant_buffer_;

	Buffer*			vertex_buffer_;
	Buffer*			index_buffer_;

	Matrix			transform_;

	static Color	white_;

	void create_vertex_buffer();
	void create_index_buffer();

	void draw( const Rect*, Texture*, const Rect*, const Color* );

public:
	Direct3D11Sprite( Direct3D* );
	~Direct3D11Sprite();

	void begin();

	void set_transform( const Matrix& );

	void draw( const Rect&, Texture*, const Rect&, const Color& = white_ );
	void draw( const Rect&, Texture*, const Color& = white_ );

	void draw( Texture*, const Rect&, const Color& = white_ );

	void end();

}; // class Direct3D11Sprite

#endif // DIRECT_3D_11_SPRITE_H
