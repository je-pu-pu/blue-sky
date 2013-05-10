#include "Direct3D11GraphicsManager.h"
#include "Direct3D11Mesh.h"

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

} // namespace blue_sky