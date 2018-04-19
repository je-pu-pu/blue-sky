#ifndef GAME_MESH_H
#define GAME_MESH_H

#include <type/type.h>

#include <common/safe_ptr.h>

#include <vector>

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
	class SkinningInfo;
	typedef std::vector< SkinningInfo > SkinningInfoList;

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

/**
 * ���_���̃X�L�j���O���
 *
 */
class Mesh::SkinningInfo
{
public:
	typedef u8_t BoneIndexList[ 4 ];
	typedef u8_t WeightList[ 4 ];
		
private:
	BoneIndexList	bone_index_list_;	///< �{�[���C���f�b�N�X ( 0 .. 3 )
	WeightList		weight_list_;		///< �E�G�C�g ( 0 .. 3 )
	
public:
	SkinningInfo()
	{
		for ( int n = 0; n < 4; ++n )
		{
			bone_index_list_[ n ] = 0;
			weight_list_[ n ] = 0;
		}
	}

	void add( int bone_index, float weight )
	{
		assert( bone_index >= 0 );
		assert( bone_index <= 255 );
		assert( weight >= 0.f );
		assert( weight <= 1.f );

		for ( int n = 0; n < 4; ++n )
		{
			if ( bone_index_list_[ n ] == 0 && weight_list_[ n ] == 0 )
			{
				bone_index_list_[ n ] = static_cast< u8_t >( bone_index );
				weight_list_[ n ] = static_cast< u8_t >( weight * 255 );

				return;
			}
		}

		assert( false );
	}
};

} // namespace game

#endif // GAME_MESH_H
