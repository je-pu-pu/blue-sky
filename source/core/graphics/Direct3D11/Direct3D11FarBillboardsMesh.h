#pragma once

#include "Direct3D11Mesh.h"

/**
 * ���i�r���{�[�h���b�V��
 *
 */
class Direct3D11FarBillboardsMesh : public Direct3D11Mesh 
{
public:

protected:
	void create_index_buffer( Material* );

public:
	Direct3D11FarBillboardsMesh( Direct3D11* );
	~Direct3D11FarBillboardsMesh() { }

	bool has_animation() const { return false; }

}; // class Direct3D11FarBillboardsMesh
