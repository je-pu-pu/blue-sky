#ifndef DIRECT_3D_9_H
#define DIRECT_3D_9_H

#include <d3dx9.h>

/**
 * Direct3D 9 のラッパークラス
 *
 */
class Direct3D9
{
private:
	LPDIRECT3D9				direct_3d_;		///< Direct3D 9 
	LPDIRECT3DDEVICE9		device_;		///< Direct3D 9 Device
	LPDIRECT3DVERTEXBUFFER9	vertex_buffer_;	///< Direct3D 9 Vertex Buffer

public:
	Direct3D9( HWND );
	~Direct3D9();
	
	LPDIRECT3DDEVICE9 getDevice() const { return device_; }
	LPDIRECT3DVERTEXBUFFER9	getVertexBuffer() const { return vertex_buffer_; }

}; // class Direct3D9

#endif // DIRECT_3D_H
