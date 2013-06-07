#ifndef BLUE_SKY_DRAWING_MESH_H
#define BLUE_SKY_DRAWING_MESH_H

#include "type.h"
#include <common/auto_ptr.h>
#include <game/GraphicsManager.h>

class SkinningAnimationSet;

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
	// template < typename T > using auto_ptr = typedef common::auto_ptr< T >;

private:
	common::auto_ptr< Mesh > mesh_;

public:
	DrawingMesh( Mesh* );

	bool load_obj( const char_t* );
	bool load_fbx( const char_t*, common::safe_ptr< SkinningAnimationSet >& );

	void render() const;

}; // class DrawingMesh

} // namespace blue_sky

#endif // BLUE_SKY_DRAWING_MESH_H
