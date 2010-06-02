#ifndef DIRECT_3D_9_MESH_H
#define DIRECT_3D_9_MESH_H

#include <d3dx9.h>

class Direct3D9;

/**
 * Direct3D 9 のラッパークラス
 *
 */
class Direct3D9Mesh
{
private:
	Direct3D9* direct_3d_;
	LPD3DXMESH mesh_;
	DWORD material_count_;

public:
	Direct3D9Mesh( Direct3D9* );
	~Direct3D9Mesh();

	bool loadX( const char* );
	void render();

}; // class Direct3D9

#endif // DIRECT_3D_9_MESH_H
