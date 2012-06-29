#ifndef DIRECT_3D_11_MESH_H
#define DIRECT_3D_11_MESH_H

#include <d3d11.h>
#include <xnamath.h>
#include <vector>

class Direct3D11;

/**
 * obj ファイルから読み込むメッシュ
 *
 */
class Direct3D11Mesh
{
public:
	typedef XMFLOAT3 Vertex;
	typedef WORD Index;

	typedef std::vector< Vertex > VertexList;
	typedef std::vector< Index > IndexList;

	static const DXGI_FORMAT IndexBufferFormat = DXGI_FORMAT_R16_UINT;

protected:
	Direct3D11*		direct_3d_;
	
	ID3D11Buffer*	vertex_buffer_;
	ID3D11Buffer*	index_buffer_;

	VertexList		vertex_list_;
	IndexList		index_list_;
	
	void create_vertex_buffer();
	void create_index_buffer();

	/*
	LPDIRECT3DTEXTURE9* textures_;

	virtual LPDIRECT3DTEXTURE9 load_texture( const char* ) const ;
	virtual std::string get_texture_file_name_by_texture_name( const char* ) const;
	*/

public:
	Direct3D11Mesh( Direct3D11* );
	virtual ~Direct3D11Mesh();

	bool load_obj( const char* );
	

	void render() const;

}; // class Direct3D11Mesh

#endif // DIRECT_3D_11_MESH_H
