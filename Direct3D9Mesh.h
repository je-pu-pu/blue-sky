#ifndef DIRECT_3D_9_MESH_H
#define DIRECT_3D_9_MESH_H

#include <d3dx9.h>
#include <string>

class Direct3D9;

/**
 * Direct3D 9 のラッパークラス
 *
 */
class Direct3D9Mesh
{
protected:
	Direct3D9* direct_3d_;
	LPD3DXMESH mesh_;
	
	DWORD material_count_;
	
	D3DMATERIAL9* materials_;
	LPDIRECT3DTEXTURE9* textures_;

	virtual LPDIRECT3DTEXTURE9 load_texture( const char* ) const ;
	virtual std::string get_texture_file_name_by_texture_name( const char* ) const;

public:
	Direct3D9Mesh( Direct3D9* );
	virtual ~Direct3D9Mesh();

	bool load_x( const char* );

	void render() const;

}; // class Direct3D9

#endif // DIRECT_3D_9_MESH_H
