#ifndef DIRECT_3D_11_MESH_H
#define DIRECT_3D_11_MESH_H

#include "Direct3D11Common.h"
#include <core/type.h>

#include <game/Mesh.h>

#include <common/safe_ptr.h>
#include <common/math.h>

#include <vector>
#include <cassert>

class Direct3D11;
class Direct3D11Color;
class Direct3D11Material;
class Direct3D11Matrix;

class SkinningAnimationSet;
class FbxFileLoader;

/**
 * メッシュ
 *
 */
class Direct3D11Mesh : public game::Mesh
{
public:
	typedef Direct3D11					Direct3D;
	typedef Direct3D11Color				Color;
	typedef Direct3D11Material			Material;
	typedef Direct3D11Matrix			Matrix;

	typedef DirectX::XMFLOAT2			Vector2;
	typedef DirectX::XMFLOAT3			Vector3;
	typedef DirectX::XMFLOAT4			Vector4;

public:

	/**
	 * 頂点情報
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
	 * 頂点毎のスキニング情報
	 *
	 */
	struct SkinningInfo
	{
	public:
		typedef u8_t BoneIndexList[ 4 ];
		typedef u8_t WeightList[ 4 ];
		
	private:
		BoneIndexList	bone_index_list_;	///< ボーンインデックス ( 0 .. 3 )
		WeightList		weight_list_;		///< ウエイト ( 0 .. 3 )
	
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
					bone_index_list_[ n ] = static_cast< u8_t >( bone_index );
					weight_list_[ n ] = static_cast< u8_t >( weight * 255 );

					return;
				}
			}

			assert( false );
		}
	};

	typedef WORD							Index;

	typedef Vector3							Position;
	typedef Vector3							Normal;
	typedef Vector2							TexCoord;

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

	common::safe_ptr< SkinningAnimationSet > skinning_animation_set_;

	Material* create_material();

	void optimize();

	void create_vertex_buffer();
	void create_index_buffer();

	void clear_vertex_list();
	void clear_skinning_info_list();
	void clear_index_list();

	virtual void create_index_buffer( Material* );

	virtual string_t get_texture_file_name_by_texture_name( const char_t* ) const;

public:
	Direct3D11Mesh( Direct3D* );
	virtual ~Direct3D11Mesh();

	bool load_obj( const char_t* );
	bool load_fbx( const char_t*, FbxFileLoader* );
	bool load_fbx( const char_t*, FbxFileLoader*, common::safe_ptr< SkinningAnimationSet >& );
	
	Material* get_material_at( int, bool force = true );
	Material* get_material_at_last( bool force = true );

	game::Material* get_material_at( uint_t ) override;
	uint_t get_material_count() const override { return material_list_.size(); }

	SkinningAnimationSet* setup_skinning_animation_set();
	
	SkinningAnimationSet* get_skinning_animation_set() { return skinning_animation_set_.get(); }
	const SkinningAnimationSet* get_skinning_animation_set() const { return skinning_animation_set_.get(); }

	void bind_to_ia() const override;
	void render() const override;

	inline VertexList& get_vertex_list() { return vertex_list_; }
	inline const VertexList& get_vertex_list() const { return vertex_list_; }

	inline SkinningInfoList& get_skinning_info_list() { return skinning_info_list_; }
	inline const SkinningInfoList& get_skinning_info_list() const { return skinning_info_list_; }

	inline MaterialList& get_material_list() { return material_list_; }
	inline const MaterialList& get_material_list() const { return material_list_; }

}; // class Direct3D11Mesh

#endif // DIRECT_3D_11_MESH_H
