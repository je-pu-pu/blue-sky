#ifndef GAME_MESH_H
#define GAME_MESH_H

#include <type/type.h>
#include <common/safe_ptr.h>

/// @todo �ˑ��֌W���Ȃ�Ƃ�����
class SkinningAnimationSet;
class FbxFileLoader;

namespace game
{

class Material;

/**
 * ���̕�
 *
 */
class Mesh
{
public:
	Mesh() { }
	virtual ~Mesh() { }

	virtual bool load_obj( const char_t* ) { return false; }

	/** @todo FBX �� Mesh �ȊO���܂ނ��߁AMesh::load_fbx() �ł͂Ȃ� FbxFileLoader::load() �� SceneGraph ��Ԃ��悤�ɂ���H */
	virtual bool load_fbx( const char_t* ) { return false; }
	virtual bool load_fbx( const char_t*, FbxFileLoader* ) { return false; }
	virtual bool load_fbx( const char_t*, FbxFileLoader*, common::safe_ptr< SkinningAnimationSet >& ) { return false; }

	virtual Material* get_material_at( uint_t ) = 0;
	virtual uint_t get_material_count() const = 0;

	virtual void bind_to_ia() const = 0;
	virtual void render() const = 0;
};

} // namespace game

#endif // GAME_MESH_H
