#ifndef DIRECT_3D_11_AXIS_H
#define DIRECT_3D_11_AXIS_H

#include <d3d11.h>
#include <xnamath.h>
#include <vector>

class Direct3D11;

/**
 * �f�o�b�O�\���p�� XYZ ��
 *
 */
class Direct3D11Axis
{
public:
	struct Vertex
	{
		XMFLOAT3 Position;
		XMFLOAT3 Color;
	};

	typedef XMFLOAT3 Position;
	typedef XMFLOAT3 Color;

protected:
	Direct3D11*		direct_3d_;
	ID3D11Buffer*	vertex_buffer_;

	void create_vertex_buffer();

public:
	Direct3D11Axis( Direct3D11* );
	virtual ~Direct3D11Axis();

	void render() const;

}; // class Direct3D11Axis

#endif // DIRECT_3D_11_AXIS_H
