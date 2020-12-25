#pragma once

#include "BaseFileLoader.h"
#include <blue_sky/graphics/Mesh.h>
#include <core/animation/Animation.h>
#include <fbxsdk.h>
#include <unordered_map>

// FBX SDK 2020.2 fbxarch.h ‚É‚æ‚é #define snprintf _snprintf ‚ðŒ³‚É–ß‚·
#undef snprintf

namespace blue_sky::graphics
{

/**
 * FBX File Loader
 *
 */
class FbxFileLoader : public BaseFileLoader
{
public:
	using Animation			= core::Animation;
	using AnimationKeyFrame	= core::AnimationKeyFrame;

private:
	FbxManager* fbx_manager_;
	FbxScene*	fbx_scene_;
	int			fbx_material_index_;
	std::unordered_map< FbxNode*, uint_t > bone_index_map_;

protected:
	void load_mesh_recursive( FbxNode* );
	
	void load_mesh( FbxMesh* );
	void load_mesh_vertex_weight( FbxSkin*, Mesh::VertexWeightList& );
	void load_material( FbxSurfaceMaterial* );
	
	void load_limb_recursive( FbxNode* );
	void load_limb( FbxNode* );

	void load_animations();
	void load_animations_for_bone( int, FbxNode* );
	void load_curve_for_animation( Animation&, Animation::ChannelIndex, const FbxAnimCurve* );

	void convert_coordinate_system();

	void set_relative_file_name_to_texture_file_name_recursive( FbxNode* );

	static string_t convert_file_path_to_internal_encoding( const char* );

	void print_local_transform( const FbxNode* ) const;
	void print_matrix( const FbxAMatrix& ) const;
	void print_axis_system( const FbxAxisSystem& ) const;

public:
	FbxFileLoader( Model*, const char_t* );
	~FbxFileLoader();

	bool load( const char_t* ) override;

	bool load_fbx( const char_t* file_path );
	bool save_fbx( const char_t* file_path );
	
	bool convert_to_binaly( const char_t* file_path, const char_t* binary_file_path );

}; // class FbxFileLoader

} // namespace blue_sky::graphics
