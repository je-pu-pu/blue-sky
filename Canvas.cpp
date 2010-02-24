#include "Canvas.h"
#include <math.h>

namespace art
{

void Canvas::Vertex::update()
{
	static float a = 0.f;
	a += 0.01f;

	art::Vertex d = target_vertex() - vertex();
	direction() = ( direction() * 31 + d ) / 32;

	// direction() *= ( sin( a ) + 1.f ) * 0.1f;

	vertex_ += direction_;
}

}; // namespace art