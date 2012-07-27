#ifndef BLUE_SKY_CONSTANT_BUFFER_H
#define BLUE_SKY_CONSTANT_BUFFER_H

#include "Direct3D11Color.h"

namespace blue_sky
{

struct GameConstantBuffer
{
	XMMATRIX projection;
};

struct FrameConstantBuffer
{
	XMMATRIX view;
	XMMATRIX shadow_view_projection;
	float time;
};

struct ObjectConstantBuffer
{
	XMMATRIX world;
	Direct3D11Color color;

	ObjectConstantBuffer()
		: color( 0.f, 0.f, 0.f, 0.f )
	{ }
};

} // namespace blue_sky

#endif // BLUE_SKY_CONSTANT_BUFFER_H