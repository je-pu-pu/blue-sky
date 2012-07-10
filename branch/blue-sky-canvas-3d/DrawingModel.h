#ifndef DRAWING_MODEL_H
#define DRAWING_MODEL_H

#include <d3d11.h>
#include <xnamath.h>
#include <vector>

class Direct3D11;

/**
 * obj ファイルから読み込む手描きモデル
 *
 */
class DrawingModel
{
public:
	struct Vertex
	{
		XMFLOAT3 Position;
		XMFLOAT4 Color;

		bool operator < ( const Vertex& v ) const
		{
			if ( Position.x < v.Position.x ) return true;
			if ( Position.x > v.Position.x ) return false;
			if ( Position.y < v.Position.y ) return true;
			if ( Position.y > v.Position.y ) return false;
			if ( Position.z < v.Position.z ) return true;
			if ( Position.z > v.Position.z ) return false;

			if ( Color.x < v.Color.x ) return true;
			if ( Color.x > v.Color.x ) return false;
			if ( Color.y < v.Color.y ) return true;
			if ( Color.y > v.Color.y ) return false;

			return false;
		}
	};

	typedef XMFLOAT3 Position;
	typedef XMFLOAT4 Color;

	typedef WORD Index;

	typedef std::vector< Position > PositionList;
	typedef std::vector< Color > ColorList;

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
	DrawingModel( Direct3D11* );
	virtual ~DrawingModel();

	bool load_obj( const char* );

	void render() const;

}; // class DrawingModel

#endif // DRAWING_MODEL_H
