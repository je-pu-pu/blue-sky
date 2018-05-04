#pragma once

#include "Direct3D11Common.h"
#include <core/type.h>

#include <game/Mesh.h>
#include <game/VertexGroup.h>
#include <game/Material.h>
#include <game/Texture.h>

#include <common/safe_ptr.h>
#include <common/math.h>

#include <memory>
#include <vector>
#include <cassert>

class Direct3D11;

class SkinningAnimationSet;
class FbxFileLoader;

/**
 * ÉÅÉbÉVÉÖ
 *
 */
class Direct3D11Mesh : public game::Mesh
{
public:
	typedef Direct3D11					Direct3D;
	typedef game::VertexGroup			VertexGroup;
	typedef game::Material				Material;
	typedef game::Texture				Texture;

public:

	/**
	 * í∏ì_èÓïÒ
	 *
	 * @todo game::Mesh or blue_sky::Mesh Ç…à⁄ìÆÇ∑ÇÈ
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
			return
				( Position <  v.Position ) ||
				( Position == v.Position && Normal <  v.Normal ) ||
				( Position == v.Position && Normal == v.Normal && TexCoord < v.TexCoord );
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
	typedef std::vector< std::unique_ptr< VertexGroup > > VertexGroupList;
	typedef std::vector< std::unique_ptr< Material > > MaterialList;

	typedef std::vector< VertexGroup::IndexList::size_type > IndexCountList;
	
	typedef std::vector< com_ptr< ID3D11Buffer > >	BufferList;

	static const DXGI_FORMAT IndexBufferFormat = DXGI_FORMAT_R16_UINT;

protected:
	Direct3D*			direct_3d_;
	BufferList			vertex_buffer_list_;
	BufferList			index_buffer_list_;

	VertexList			vertex_list_;
	SkinningInfoList	skinning_info_list_;

	IndexCountList		index_count_list_;

	VertexGroupList		vertex_group_list_;
	MaterialList		material_list_;

	std::unique_ptr< SkinningAnimationSet > skinning_animation_set_;

	Material* create_material();

	void optimize();

	void create_vertex_buffer();
	void create_index_buffer();

	void clear_vertex_list();
	void clear_vertex_group_list();
	void clear_skinning_info_list();

public:
	Direct3D11Mesh( Direct3D* );
	virtual ~Direct3D11Mesh();

	/// @todo êÆóùÇ∑ÇÈ
	bool load_obj( const char_t* );
	bool load_fbx( const char_t*, FbxFileLoader* );
	bool load_fbx( const char_t*, FbxFileLoader*, common::safe_ptr< SkinningAnimationSet >& );
	
	/// @todo Loader Ç™Ç‚ÇÈÇÊÇ§Ç…Ç∑ÇÈ
	virtual string_t get_texture_file_path_by_texture_name( const char_t* ) const;
	Texture* load_texture_by_texture_name( const char_t* ) const;

	VertexGroup* create_vertex_group() override;
	VertexGroup* get_vertex_group_at( uint_t, bool force );

	Material* get_material_at( uint_t, bool force );

	Material* get_material_at( uint_t ) override;
	uint_t get_material_count() const override { return material_list_.size(); }
	void set_material_at( uint_t n, Material* m ) override { material_list_[ n ].reset( m ); }

	SkinningAnimationSet* setup_skinning_animation_set();
	
	SkinningAnimationSet* get_skinning_animation_set() { return skinning_animation_set_.get(); }
	const SkinningAnimationSet* get_skinning_animation_set() const { return skinning_animation_set_.get(); }

	void flip() override;

	void bind() const override;
	void render() const override;

	inline VertexList& get_vertex_list() { return vertex_list_; }
	inline const VertexList& get_vertex_list() const { return vertex_list_; }

	inline SkinningInfoList& get_skinning_info_list() { return skinning_info_list_; }
	inline const SkinningInfoList& get_skinning_info_list() const { return skinning_info_list_; }

	inline MaterialList& get_material_list() { return material_list_; }
	inline const MaterialList& get_material_list() const { return material_list_; }

}; // class Direct3D11Mesh
