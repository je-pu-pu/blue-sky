#ifndef DIRECT_3D_9_MESH_MANAGER_H
#define DIRECT_3D_9_MESH_MANAGER_H

#include <map>
#include <string>

class Direct3D9;
class Direct3D9Mesh;

/**
 * ƒƒbƒVƒ…ŠÇ—
 *
 */
class Direct3D9MeshManager
{
public:
	typedef Direct3D9Mesh Mesh;
	typedef std::map< std::string, Mesh* > MeshList;

private:
	Direct3D9* direct_3d_;
	MeshList mesh_list_;

public:
	Direct3D9MeshManager( Direct3D9* );
	~Direct3D9MeshManager();

	Mesh* load( const char*, const char* );
	Mesh* get( const char* ) const;

	void unload( const char* );
	void unload_all();

}; // class Direct3D9MeshManager

#endif // DIRECT_3D_9_MESH_MANAGER_H