#ifndef DIRECT_3D_9_BOX_H
#define DIRECT_3D_9_BOX_H

#include <d3dx9.h>

class Direct3D9;

/**
 * Direct3D 9 BOX
 *
 */
class Direct3D9Box
{
private:
	Direct3D9* direct_3d_;

	LPDIRECT3DVERTEXBUFFER9 position_vertex_buffer_;
	LPDIRECT3DVERTEXBUFFER9 color_vertex_buffer_;

	LPDIRECT3DINDEXBUFFER9 index_buffer_;

public:
	Direct3D9Box( Direct3D9*, float, float, float );
	~Direct3D9Box();

	void ready();
	void render();

}; // class Direct3D9Box

#endif // DIRECT_3D_9_BOX_H
