#ifndef BLUE_SKY_DRAWING_MESH_H
#define BLUE_SKY_DRAWING_MESH_H

#include "type.h"
#include <game/Mesh.h>
#include <game/Material.h>
#include <common/safe_ptr.h>
#include <common/auto_ptr.h>

class SkinningAnimationSet;
class FbxFileLoader;

namespace blue_sky
{

/**
 * è•`‚«•—ƒƒbƒVƒ…
 *
 */
class DrawingMesh
{
public:
	typedef game::Mesh Mesh;
	typedef game::Material Material;
	// template < typename T > using auto_ptr = typedef common::auto_ptr< T >;

private:
	common::auto_ptr< Mesh > mesh_;

public:
	DrawingMesh( Mesh* );
	~DrawingMesh();

	bool load_obj( const char_t* );
	bool load_fbx( const char_t*, FbxFileLoader*, common::safe_ptr< SkinningAnimationSet >& );

	Material* get_material_at( uint_t );

	void render() const;

}; // class DrawingMesh

} // namespace blue_sky

#endif // BLUE_SKY_DRAWING_MESH_H
