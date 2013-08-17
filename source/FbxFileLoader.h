#ifndef FBX_FILE_LOADER_H
#define FBX_FILE_LOADER_H

#include "type.h"
#include "Animation.h"
#include "Direct3D11Mesh.h"
#include <fbxsdk.h>
#include <map>

namespace game
{

class Mesh;

}

/**
 * FBX File Loader
 *
 */
class FbxFileLoader
{
public:
	typedef Direct3D11Mesh Mesh;

private:
	FbxManager* fbx_manager_;
	Mesh*		mesh_;
	FbxScene*	fbx_scene_;
	int			fbx_material_index_;
	std::map< FbxNode*, uint_t > bone_index_map_;

protected:
	void load_mesh_recursive( FbxNode* );
	
	void load_mesh( FbxMesh* );
	void load_mesh_skinning_info( FbxSkin*, Mesh::SkinningInfoList& );
	void load_material( FbxSurfaceMaterial* );
	
	void load_limb_recursive( FbxNode* );
	void load_limb( FbxNode* );

	void load_animations();
	void load_animations_for_bone( int, FbxNode* );
	void load_curve_for_animation( Animation&, Animation::ChannelIndex, const FbxAnimCurve* );

	void convert_coordinate_system();

	static string_t convert_file_path_to_internal_encoding( const char* );

	void print_local_transform( const FbxNode* ) const;
	void print_matrix( const FbxAMatrix& ) const;
	void print_axis_system( const FbxAxisSystem& ) const;

public:
	FbxFileLoader();
	~FbxFileLoader();

	bool load( Mesh*, const char_t* );
	bool save( const char_t* );
	
}; // class FbxFileLoader

#endif // FBX_FILE_LOADER_H
