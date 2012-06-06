#ifndef DIRECT_3D_9_RECTANGLE_H
#define rectangle

#include <d3dx9.h>

class Direct3D9;

/**
 * Direct3D 9 Rectangle
 *
 */
class Direct3D9Rectangle
{
private:
	Direct3D9* direct_3d_;

	LPDIRECT3DVERTEXDECLARATION9  vertex_declaration_;

	LPDIRECT3DVERTEXBUFFER9 position_vertex_buffer_;
	LPDIRECT3DVERTEXBUFFER9 color_vertex_buffer_;
	LPDIRECT3DVERTEXBUFFER9 uv_vertex_buffer_;

	LPDIRECT3DINDEXBUFFER9 index_buffer_;

public:
	Direct3D9Rectangle( Direct3D9* );
	~Direct3D9Rectangle();

	void ready();
	void render();

}; // class Direct3D9Rectangle

#endif // DIRECT_3D_9_RECTANGLE_H
