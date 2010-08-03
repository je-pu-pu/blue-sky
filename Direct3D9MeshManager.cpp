#include "Direct3D9MeshManager.h"
#include "Direct3D9Mesh.h"
#include "Direct3D9.h"
#include "DirectX.h"

#include <common/exception.h>
#include <common/serialize.h>
#include <common/log.h>

#include <string>

Direct3D9MeshManager::Direct3D9MeshManager( Direct3D9* direct_3d )
	: direct_3d_( direct_3d )
{

}

Direct3D9MeshManager::~Direct3D9MeshManager()
{
	unload_all();
}

Direct3D9MeshManager::Mesh* Direct3D9MeshManager::load( const char* name, const char* file_name )
{
	MeshList::iterator i = mesh_list_.find( name );

	if ( i != mesh_list_.end() )
	{
		return i->second;
	}

	Mesh* mesh = new Direct3D9Mesh( direct_3d_ );
	mesh_list_[ name ] = mesh;

	mesh->load_x( file_name );

	return mesh;
}

Direct3D9MeshManager::Mesh* Direct3D9MeshManager::get( const char* name ) const
{
	MeshList::const_iterator i = mesh_list_.find( name );

	if ( i != mesh_list_.end() )
	{
		return i->second;
	}

	return 0;
}

void Direct3D9MeshManager::unload( const char* name )
{
	MeshList::iterator i = mesh_list_.find( name );

	if ( i != mesh_list_.end() )
	{
		delete i->second;
		mesh_list_.erase( i );
	}
}

void Direct3D9MeshManager::unload_all()
{
	for ( MeshList::iterator i = mesh_list_.begin(); i != mesh_list_.end(); ++i )
	{
		delete i->second;
	}

	mesh_list_.clear();
}