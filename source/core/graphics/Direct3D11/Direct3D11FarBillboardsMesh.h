#ifndef DIRECT_3D_11_FAR_BILLBOARDS_MESH_H
#define DIRECT_3D_11_FAR_BILLBOARDS_MESH_H

#include "Direct3D11Mesh.h"

/**
 * 遠景ビルボードメッシュ
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

#endif // DIRECT_3D_11_FAR_BILLBOARDS_MESH_H
