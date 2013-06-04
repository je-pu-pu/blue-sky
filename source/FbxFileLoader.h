#ifndef FBX_FILE_LOADER_H
#define FBX_FILE_LOADER_H

#include "type.h"
#include "Direct3D11Mesh.h"
#include <fbxsdk.h>

namespace game
{

class Mesh;

}

class Animation;

/**
 * FBX File Loader
 *
 */
class FbxFileLoader
{
public:
	typedef Direct3D11Mesh Mesh;
	typedef Direct3D11Material Material;

private:
	Mesh*		mesh_;
	FbxScene*	fbx_scene_;
	int			fbx_material_index_;

protected:
	void load_node_recursive( FbxNode* );
	
	void load_mesh( FbxMesh* );
	void load_material( FbxSurfaceMaterial* );
	
	void load_animations_for_bone( int, FbxCluster* );
	void load_curve_for_animation( Animation&, const char_t*, const FbxAnimCurve* );

	void convert_coordinate_system();

public:
	FbxFileLoader( Mesh* );

	bool load( const char_t* );
	
}; // class FbxFileLoader

#endif // FBX_FILE_LOADER_H
