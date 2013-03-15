#ifndef BLUE_SKY_CONSTANT_BUFFER_H
#define BLUE_SKY_CONSTANT_BUFFER_H

#include "Direct3D11ConstantBuffer.h"
#include "Direct3D11Color.h"

namespace blue_sky
{

template< int N >
struct BaseConstantBufferData
{
	static const int DEFAULT_SLOT = N;
};

struct GameConstantBufferData : public BaseConstantBufferData< 0 >
{
	XMMATRIX projection;
	float_t screen_width;
	float_t screen_height;
};

struct FrameConstantBufferData : public BaseConstantBufferData< 1 >
{
	XMMATRIX view;
	float_t time;
};

struct ObjectConstantBufferData : public BaseConstantBufferData< 2 >
{
	XMMATRIX world;
	Direct3D11Color color;

	ObjectConstantBufferData()
		: color( 0.f, 0.f, 0.f, 0.f )
	{ }
};

struct ObjectExtentionConstantBufferData : public BaseConstantBufferData< 3 >
{
	Direct3D11Color color;
};

struct FrameDrawingConstantBufferData : public BaseConstantBufferData< 4 >
{
	float_t accent;
	float_t a, b, c;
};

typedef Direct3D11ConstantBuffer< GameConstantBufferData > GameConstantBuffer;
typedef Direct3D11ConstantBuffer< FrameConstantBufferData > FrameConstantBuffer;
typedef Direct3D11ConstantBuffer< FrameDrawingConstantBufferData > FrameDrawingConstantBuffer;
typedef Direct3D11ConstantBuffer< ObjectConstantBufferData > ObjectConstantBuffer;
typedef Direct3D11ConstantBuffer< ObjectExtentionConstantBufferData > ObjectExtentionConstantBuffer;

} // namespace blue_sky

#endif // BLUE_SKY_CONSTANT_BUFFER_H