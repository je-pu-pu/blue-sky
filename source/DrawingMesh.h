#ifndef BLUE_SKY_DRAWING_MESH_H
#define BLUE_SKY_DRAWING_MESH_H

#include "type.h"
#include <game/GraphicsManager.h>

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

private:
	Mesh* mesh_;

public:
	DrawingMesh( Mesh* );

	bool load_obj( const char_t* );
	bool load_fbx( const char_t* );

	void render() const;

}; // class DrawingMesh

} // namespace blue_sky

#endif // BLUE_SKY_DRAWING_MESH_H
