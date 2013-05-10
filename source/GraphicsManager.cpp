#include "GraphicsManager.h"
#include "FbxFileLoader.h"

#include "DrawingMesh.h"
#include "DrawingLine.h"

namespace blue_sky
{

/**
 * ��`�������b�V����ǂݍ���
 *
 * @param name ���O
 * @return ��`�������b�V���A�܂��͎��s���� 0 ��Ԃ�
 */
DrawingMesh* GraphicsManager::load_drawing_mesh( const char_t* name )
{
	DrawingMesh* mesh = create_drawing_mesh();
	
	if ( ! mesh->load_fbx( ( string_t( "media/model/" ) + name + ".fbx" ).c_str() ) )
	{
		mesh->load_obj( ( string_t( "media/model/" ) + name + ".obj" ).c_str() );
	}
	
	return mesh;
}

/**
 * ��`�������C����ǂݍ���
 *
 * @param name ���O
 * @return ��`�������C���A�܂��͎��s���� 0 ��Ԃ�
 */
DrawingLine* GraphicsManager::load_drawing_line( const char_t* name )
{
	DrawingLine* line = create_drawing_line();
	line->load_obj( ( string_t( "media/model/" ) + name + "-line.obj" ).c_str() );

	return line;
}

}