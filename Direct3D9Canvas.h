#ifndef ART_DIRECT_3D_9_CANVAS_H
#define ART_DIRECT_3D_9_CANVAS_H

#include "Canvas.h"
#include "Direct3D9.h"

class Direct3D9;

namespace art
{

/**
 * 
 */
class Direct3D9Canvas : public Canvas
{
public:
	struct Vertex
	{
		

	}; // class Vertex

private:
	Direct3D9*	direct_3d_;
	LPD3DXFONT	font_;
public:
	Direct3D9Canvas( HWND );
	virtual ~Direct3D9Canvas();

	void render() const;

	void drawLineHumanTouch( const art::Vertex&, const art::Vertex&, const Color& );
	void drawPolygonHumanTouch( const Face&, const Color& );
	void fillRect( const Rect&, const Color& );

	virtual void drawLine( Real, Real, Real, Real, const Color& );
	virtual void drawCircle( const art::Vertex&, Real, const Color&, bool );
	virtual void drawRing( const art::Vertex&, Real, Real, const Color&, bool );

	virtual void drawText( const art::Vertex&, const char*, const Color& );

	virtual void begin() const;
	virtual void end() const;

	static void BeginLine();
	static void EndLine();
};

}

#endif // ART_DIRECT_3D_9_CANVAS_H