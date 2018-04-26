#pragma once

#include "Direct3D11Common.h"
#include <DirectXMath.h>
#include <vector>

class Direct3D11;

/**
 * デバッグ表示用の XYZ 軸
 *
 */
class Direct3D11Axis
{
public:
	struct Vertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Color;
	};

	typedef DirectX::XMFLOAT3 Position;
	typedef DirectX::XMFLOAT3 Color;

protected:
	Direct3D11*		direct_3d_;
	ID3D11Buffer*	vertex_buffer_;

	void create_vertex_buffer();

public:
	Direct3D11Axis( Direct3D11* );
	virtual ~Direct3D11Axis();

	void render() const;

}; // class Direct3D11Axis
