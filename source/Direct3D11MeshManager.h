#ifndef DIRECT_3D_11_MESH_MANAGER_H
#define DIRECT_3D_11_MESH_MANAGER_H

#include <map>
#include <string>

class Direct3D11;
class Direct3D11Mesh;

/**
 * ƒƒbƒVƒ…ŠÇ—
 *
 */
class Direct3D11MeshManager
{
public:
	typedef Direct3D11Mesh Mesh;
	typedef std::map< std::string, Mesh* > MeshList;

private:
	Direct3D11* direct_3d_;
	MeshList mesh_list_;

public:
	Direct3D11MeshManager( Direct3D11* );
	~Direct3D11MeshManager();

	Mesh* load( const char*, const char* );
	Mesh* get( const char* ) const;

	void unload( const char* );
	void unload_all();

}; // class Direct3D11MeshManager

#endif // DIRECT_3D_11_MESH_MANAGER_H