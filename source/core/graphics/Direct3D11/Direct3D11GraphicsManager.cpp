#include "Direct3D11GraphicsManager.h"
#include "Direct3D11Mesh.h"
#include "Direct3D11TextureManager.h"
#include "Direct3D11.h"


#include "ConstantBuffer.h"
#include "DrawingMesh.h"
#include "DrawingLine.h"

namespace blue_sky
{

/**
 * �R���X�g���N�^
 *
 * @param direct_3d Direct3D
 */
Direct3D11GraphicsManager::Direct3D11GraphicsManager( Direct3D* direct_3d )
	: direct_3d_( direct_3d )
	, game_render_data_( new GameConstantBuffer( direct_3d ) )
	, frame_render_data_( new FrameConstantBuffer( direct_3d ) )
	, frame_drawing_render_data_( new FrameDrawingConstantBuffer( direct_3d ) )
	, shared_object_render_data_( new ObjectConstantBuffer( direct_3d ) )
{

}

/**
 * �f�X�g���N�^
 *
 */
Direct3D11GraphicsManager::~Direct3D11GraphicsManager()
{
	
}

/**
 * ���b�V���𐶐�����
 *
 * @return ���b�V��
 */
Direct3D11GraphicsManager::Mesh* Direct3D11GraphicsManager::create_mesh()
{
	return new Direct3D11Mesh( direct_3d_ );
}

/**
 * ���C���𐶐�����
 *
 * @return ���C��
 */
Direct3D11GraphicsManager::Line* Direct3D11GraphicsManager::create_line()
{
	return create_drawing_line();
}

/**
 * ��`�������b�V���𐶐�����
 *
 * @return ��`�������b�V��
 */
DrawingMesh* Direct3D11GraphicsManager::create_drawing_mesh()
{
	return new DrawingMesh( create_mesh() );
}

/**
 * ��`�������C���𐶐�����
 *
 * @return ���C��
 */
DrawingLine* Direct3D11GraphicsManager::create_drawing_line()
{
	return new DrawingLine( direct_3d_ );
}

/**
 * �e�N�X�`����ǂݍ���
 *
 * @param name �e�N�X�`����
 * @param file_path �e�N�X�`���t�@�C���p�X
 */
Direct3D11GraphicsManager::Texture* Direct3D11GraphicsManager::load_texture( const char_t* name, const char_t* file_path )
{
	return direct_3d_->getTextureManager()->load( name, file_path );
}

/**
 * �w�肵�����O�̃e�N�X�`�����擾����
 *
 * @param name �e�N�X�`����
 */
Direct3D11GraphicsManager::Texture* Direct3D11GraphicsManager::get_texture( const char_t* name )
{
	return direct_3d_->getTextureManager()->load( name, ( string_t( "media/model/" ) + name + ".png" ).c_str() );
}

/**
 * �w�肵�����O�̃e�N�X�`�����A�����[�h����
 *
 * @param name �e�N�X�`����
 */
void Direct3D11GraphicsManager::unload_texture( const char_t* name )
{
	direct_3d_->getTextureManager()->unload( name );
}

/**
 * �S�Ẵe�N�X�`�����A�����[�h����
 *
 */
void Direct3D11GraphicsManager::unload_texture_all()
{
	direct_3d_->getTextureManager()->unload_all();
}


} // namespace blue_sky