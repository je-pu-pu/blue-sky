#ifndef GAME_MESH_H
#define GAME_MESH_H

#include <type/type.h>
#include <common/safe_ptr.h>

/// @todo ˆË‘¶ŠÖŒW‚ð‚È‚ñ‚Æ‚©‚·‚é
class SkinningAnimationSet;
class FbxFileLoader;

namespace game
{

class Material;

/**
 * —§‘Ì•¨
 *
 */
class Mesh
{
public:
	Mesh() { }
	virtual ~Mesh() { }

	virtual bool load_obj( const char_t* ) { return false; }
	virtual bool load_fbx( const char_t*, FbxFileLoader*, common::safe_ptr< SkinningAnimationSet >& ) { return false; }

	virtual Material* get_material_at( uint_t ) = 0;

	virtual void render() const = 0;
};

} // namespace game

#endif // GAME_MESH_H
