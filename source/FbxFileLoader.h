#ifndef FBX_FILE_LOADER_H
#define FBX_FILE_LOADER_H

#include "type.h"

#include <fbxsdk.h>

class Direct3D11Mesh;
class Direct3D11Material;

/**
 * FBX File Loader
 *
 */
class FbxFileLoader
{
public:
	typedef Direct3D11Mesh			Mesh;
	typedef Direct3D11Material		Material;

	friend Mesh;
private:
	Mesh*	mesh_;

protected:
	void load_node_recursive( FbxNode* );

public:
	FbxFileLoader( Mesh* );

	bool load( const char_t* );

}; // class FbxFileLoader

#endif // FBX_FILE_LOADER_H
