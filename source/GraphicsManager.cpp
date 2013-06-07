#include "GraphicsManager.h"
#include "FbxFileLoader.h"

#include "DrawingMesh.h"
#include "DrawingLine.h"

namespace blue_sky
{

/**
 * 手描き風メッシュを読み込む
 *
 * @param name 名前
 * @param skinning_animation_set 同時に読み込むスキニングアニメーション情報を格納するポインタ
 * @return 手描き風メッシュ、または失敗時に 0 を返す
 */
DrawingMesh* GraphicsManager::load_drawing_mesh( const char_t* name, common::safe_ptr< SkinningAnimationSet >& skinning_animation_set )
{
	DrawingMesh* mesh = create_drawing_mesh();
	
	if ( ! mesh->load_fbx( ( string_t( "media/model/" ) + name + ".fbx" ).c_str(), skinning_animation_set ) )
	{
		mesh->load_obj( ( string_t( "media/model/" ) + name + ".obj" ).c_str() );
	}
	
	return mesh;
}

/**
 * 手描き風ラインを読み込む
 *
 * @param name 名前
 * @return 手描き風ライン、または失敗時に 0 を返す
 */
DrawingLine* GraphicsManager::load_drawing_line( const char_t* name )
{
	DrawingLine* line = create_drawing_line();
	line->load_obj( ( string_t( "media/model/" ) + name + "-line.obj" ).c_str() );

	return line;
}

}