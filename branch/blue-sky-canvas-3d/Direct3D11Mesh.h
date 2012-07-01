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
	struct Vertex
	{
		XMFLOAT3 Position;
		XMFLOAT2 TexCoord;

		bool operator < ( const Vertex& v ) const
		{
			if ( Position.x < v.Position.x ) return true;
			if ( Position.x > v.Position.x ) return false;
			if ( Position.y < v.Position.y ) return true;
			if ( Position.y > v.Position.y ) return false;
			if ( Position.z < v.Position.z ) return true;
			if ( Position.z > v.Position.z ) return false;

			if ( TexCoord.x < v.TexCoord.x ) return true;
			if ( TexCoord.x > v.TexCoord.x ) return false;
			if ( TexCoord.y < v.TexCoord.y ) return true;
			if ( TexCoord.y > v.TexCoord.y ) return false;

			return false;
		}
	};

	typedef XMFLOAT3 Position;
	typedef XMFLOAT2 TexCoord;

	typedef WORD Index;

	typedef std::vector< Position > PositionList; // ?
	typedef std::vector< TexCoord > TexCoordList; // ?

	typedef std::vector< Vertex > VertexList;
	typedef std::vector< Index > IndexList;

	static const DXGI_FORMAT IndexBufferFormat = DXGI_FORMAT_R16_UINT;

protected:
	Direct3D11*		direct_3d_;
	
	ID3D11Buffer*	vertex_buffer_;
	ID3D11Buffer*	index_buffer_;
	
	ID3D11ShaderResourceView*	texture_resource_view_;

	VertexList		vertex_list_;
	IndexList		index_list_;
	
	void create_vertex_buffer();
	void create_index_buffer();

	void create_texture_resource_view( const char* );

public:
	Direct3D11Mesh( Direct3D11* );
	virtual ~Direct3D11Mesh();

	bool load_obj( const char* );

	void render() const;

}; // class Direct3D11Mesh

#endif // DIRECT_3D_11_MESH_H
