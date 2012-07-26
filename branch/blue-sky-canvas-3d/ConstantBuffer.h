#ifndef BLUE_SKY_CONSTANT_BUFFER_H
#define BLUE_SKY_CONSTANT_BUFFER_H

#include <xnamath.h>

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
};

} // namespace blue_sky

#endif // BLUE_SKY_CONSTANT_BUFFER_H