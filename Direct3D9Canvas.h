#ifndef ART_DIRECT_3D_9_CANVAS_H
#define ART_DIRECT_3D_9_CANVAS_H

#include "Canvas.h"

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

public:
	Direct3D9Canvas( HWND );
	~Direct3D9Canvas();

	void render() const;

	void DrawLineHumanTouch( float, float, float, float, const Color& );
	void DrawPolygonHumanTouch( const Point*, const Color& );
	void FillRect( const Rect&, const Color& );
};

}

#endif // ART_DIRECT_3D_9_CANVAS_H