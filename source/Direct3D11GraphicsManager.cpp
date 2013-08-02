#include "Direct3D11GraphicsManager.h"
#include "Direct3D11Mesh.h"
#include "Direct3D11TextureManager.h"
#include "Direct3D11.h"

#include "DrawingMesh.h"
#include "DrawingLine.h"

namespace blue_sky
{

/**
 * コンストラクタ
 *
 * @param direct_3d Direct3D
 */
Direct3D11GraphicsManager::Direct3D11GraphicsManager( Direct3D* direct_3d )
	: direct_3d_( direct_3d )
{

}

/**
 * デストラクタ
 *
 */
Direct3D11GraphicsManager::~Direct3D11GraphicsManager()
{
	
}

/**
 * メッシュを生成する
 *
 * @return メッシュ
 */
Direct3D11GraphicsManager::Mesh* Direct3D11GraphicsManager::create_mesh()
{
	return new Direct3D11Mesh( direct_3d_ );
}

/**
 * ラインを生成する
 *
 * @return ライン
 */
Direct3D11GraphicsManager::Line* Direct3D11GraphicsManager::create_line()
{
	return create_drawing_line();
}

/**
 * 手描き風メッシュを生成する
 *
 * @return 手描き風メッシュ
 */
DrawingMesh* Direct3D11GraphicsManager::create_drawing_mesh()
{
	return new DrawingMesh( create_mesh() );
}

/**
 * 手描き風ラインを生成する
 *
 * @return ライン
 */
DrawingLine* Direct3D11GraphicsManager::create_drawing_line()
{
	return new DrawingLine( direct_3d_ );
}

/**
 * 指定した名前のテクスチャを取得する
 *
 * @param name テクスチャ名
 */
Direct3D11GraphicsManager::Texture* Direct3D11GraphicsManager::get_texture( const char_t* name )
{
	return direct_3d_->getTextureManager()->load( name, ( string_t( "media/model/" ) + name + ".png" ).c_str() );
}

} // namespace blue_sky