#ifndef ART_DIRECT_3D_9_CANVAS_H
#define ART_DIRECT_3D_9_CANVAS_H

#include "Canvas.h"
#include "Direct3D9.h"

#include <boost/array.hpp>

class Direct3D9;

namespace art
{

/**
 * 
 */
class Direct3D9Canvas : public Canvas
{
public:

	/**
	 * ポイントスプライト用 Vertex
	 */
	struct Vertex
	{
		static const UINT COUNT = 100000;
		static const DWORD FVF = D3DFVF_XYZRHW | D3DFVF_PSIZE | D3DFVF_DIFFUSE;

		D3DXVECTOR4		position;
		FLOAT			size;
		D3DCOLOR		color;
	}; // class Vertex

	/**
	 * 線用 Vertex
	 */
	struct VertexForLine
	{
		static const DWORD FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;

		D3DXVECTOR4		position;
		D3DCOLOR		color;
		D3DXVECTOR2		uv;
	};

	typedef IDirect3DTexture9 Texture;

private:
	Direct3D9*	direct_3d_;

	LPDIRECT3DVERTEXBUFFER9 vertex_buffer_;
	LPD3DXFONT	font_;

	Vertex* point_sprite_;
	UINT point_sprite_index_;

	boost::array< Texture*, 3 > line_texture_list_;
	boost::array< Texture*, 3 > face_texture_list_;

public:
	Direct3D9Canvas( Direct3D9* );
	virtual ~Direct3D9Canvas();

	void clear();

	void render();

	virtual void begin();
	virtual void end();

	void beginDrawLine();
	void beginDrawPolygon();

	void drawLineHumanTouch( const art::Vertex&, const art::Vertex&, const Color& );
	void drawPolygonHumanTouch( const Face&, const Color& );
	void fillRect( const Rect&, const Color& );

	virtual void drawLine( Real, Real, Real, Real, const Color& );
	virtual void drawCircle( const art::Vertex&, Real, const Color&, bool );
	virtual void drawRing( const art::Vertex&, Real, Real, const Color&, bool );

	virtual void drawText( const art::Vertex&, const char*, const Color& );

	virtual int width() const;
	virtual int height() const;
};

}

#endif // ART_DIRECT_3D_9_CANVAS_H