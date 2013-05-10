#ifndef FBX_FILE_LOADER_H
#define FBX_FILE_LOADER_H

#include "type.h"
#include "Direct3D11Mesh.h"
#include <fbxsdk.h>

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
	typedef Direct3D11Material Material;

private:
	Mesh*	mesh_;

protected:
	void load_node_recursive( FbxNode* );

public:
	FbxFileLoader( Mesh* );

	bool load( const char_t* );
	
}; // class FbxFileLoader

#endif // FBX_FILE_LOADER_H
