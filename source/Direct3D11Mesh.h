#ifndef DIRECT_3D_11_MESH_H
#define DIRECT_3D_11_MESH_H

#include "type.h"
#include <game/GraphicsManager.h>
#include <common/math.h>
#include <boost/array.hpp>
#include <d3d11.h>
#include <xnamath.h>
#include <vector>

class Direct3D11;
class Direct3D11Color;
class Direct3D11Material;

/**
 * ���b�V��
 *
 */
class Direct3D11Mesh : public game::Mesh
{
public:
	typedef Direct3D11					Direct3D;
	typedef Direct3D11Color				Color;
	typedef Direct3D11Material			Material;

	typedef XMFLOAT2					Vector2;
	typedef XMFLOAT3					Vector3;
	typedef XMFLOAT4					Vector4;

public:

	/**
	 * ���_���
	 *
	 */
	struct Vertex
	{
		Vector3 Position;
		Vector3 Normal;
		Vector2 TexCoord;

		Vertex()
			: Position( 0.f, 0.f, 0.f )
			, Normal( 0.f, 0.f, 0.f )
			, TexCoord( 0.f, 0.f )
		{

		}

		bool operator < ( const Vertex& v ) const
		{
			if ( Position.x < v.Position.x ) return true;
			if ( Position.x > v.Position.x ) return false;
			if ( Position.y < v.Position.y ) return true;
			if ( Position.y > v.Position.y ) return false;
			if ( Position.z < v.Position.z ) return true;
			if ( Position.z > v.Position.z ) return false;

			if ( Normal.x < v.Normal.x ) return true;
			if ( Normal.x > v.Normal.x ) return false;
			if ( Normal.y < v.Normal.y ) return true;
			if ( Normal.y > v.Normal.y ) return false;
			if ( Normal.z < v.Normal.z ) return true;
			if ( Normal.z > v.Normal.z ) return false;

			if ( TexCoord.x < v.TexCoord.x ) return true;
			if ( TexCoord.x > v.TexCoord.x ) return false;
			if ( TexCoord.y < v.TexCoord.y ) return true;
			if ( TexCoord.y > v.TexCoord.y ) return false;

			return false;
		}
	};

	/**
	 * ���_���̃X�L�j���O���
	 *
	 */
	struct SkinningInfo
	{
	public:
		typedef u8_t BoneIndexList[ 4 ];
		typedef u8_t WeightList[ 4 ];
		
	private:
		BoneIndexList	bone_index_list_;	///< �{�[���C���f�b�N�X ( 0 .. 3 )
		WeightList		weight_list_;		///< �E�G�C�g ( 0 .. 3 )
	
	public:
		SkinningInfo()
		{
			for ( int n = 0; n < 4; ++n )
			{
				bone_index_list_[ n ] = 0;
				weight_list_[ n ] = 0;
			}
		}

		void add( int bone_index, float weight )
		{
			assert( bone_index >= 0 );
			assert( bone_index <= 255 );
			assert( weight >= 0.f );
			assert( weight <= 1.f );

			for ( int n = 0; n < 4; ++n )
			{
				if ( bone_index_list_[ n ] == 0 && weight_list_[ n ] == 0 )
				{
					bone_index_list_[ n ] = bone_index;
					weight_list_[ n ] = static_cast< u8_t >( weight * 255 );

					return;
				}
			}

			assert( false );
		}
	};

	typedef WORD							Index;

	typedef XMFLOAT3						Position;
	typedef XMFLOAT3						Normal;
	typedef XMFLOAT2						TexCoord;

	typedef std::vector< Position >			PositionList;
	typedef std::vector< Normal >			NormalList;
	typedef std::vector< TexCoord >			TexCoordList;

	typedef std::vector< Vertex >			VertexList;
	typedef std::vector< SkinningInfo >		SkinningInfoList;
	typedef std::vector< Material* >		MaterialList;
	
	typedef std::vector< ID3D11Buffer* >	VertexBufferList;

protected:
	Direct3D*			direct_3d_;
	VertexBufferList	vertex_buffer_list_;

	VertexList			vertex_list_;
	SkinningInfoList	skinning_info_list_;

	MaterialList		material_list_;
	
	void optimize();

	void create_vertex_buffer();
	void create_index_buffer();

	virtual void create_index_buffer( Material* );

	virtual string_t get_texture_file_name_by_texture_name( const char_t* ) const;

public:
	Direct3D11Mesh( Direct3D* );
	virtual ~Direct3D11Mesh();
	
	Material* create_material();

	bool load_obj( const char_t* );
	bool load_fbx( const char_t* );
	
	void render() const;

	inline VertexList& get_vertex_list() { return vertex_list_; }
	inline const VertexList& get_vertex_list() const { return vertex_list_; }

	inline SkinningInfoList& get_skinning_info_list() { return skinning_info_list_; }
	inline const SkinningInfoList& get_skinning_info_list() const { return skinning_info_list_; }

	inline MaterialList& get_material_list() { return material_list_; }
	inline const MaterialList& get_material_list() const { return material_list_; }

}; // class Direct3D11Mesh

#endif // DIRECT_3D_11_MESH_H