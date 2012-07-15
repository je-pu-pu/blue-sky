#include "Direct3D11MeshManager.h"
#include "Direct3D11Mesh.h"
#include "Direct3D11.h"
#include "DirectX.h"

#include <common/exception.h>
#include <common/serialize.h>
#include <common/log.h>

#include <string>

Direct3D11MeshManager::Direct3D11MeshManager( Direct3D11* direct_3d )
	: direct_3d_( direct_3d )
{

}

Direct3D11MeshManager::~Direct3D11MeshManager()
{
	unload_all();
}

Direct3D11MeshManager::Mesh* Direct3D11MeshManager::load( const char* name, const char* file_name )
{
	MeshList::iterator i = mesh_list_.find( name );

	if ( i != mesh_list_.end() )
	{
		return i->second;
	}

	Mesh* mesh = new Direct3D11Mesh( direct_3d_ );
	mesh_list_[ name ] = mesh;

	mesh->load_obj( file_name );

	return mesh;
}

Direct3D11MeshManager::Mesh* Direct3D11MeshManager::get( const char* name ) const
{
	MeshList::const_iterator i = mesh_list_.find( name );

	if ( i != mesh_list_.end() )
	{
		return i->second;
	}

	return 0;
}

void Direct3D11MeshManager::unload( const char* name )
{
	MeshList::iterator i = mesh_list_.find( name );

	if ( i != mesh_list_.end() )
	{
		delete i->second;
		mesh_list_.erase( i );
	}
}

void Direct3D11MeshManager::unload_all()
{
	for ( MeshList::iterator i = mesh_list_.begin(); i != mesh_list_.end(); ++i )
	{
		delete i->second;
	}

	mesh_list_.clear();
}