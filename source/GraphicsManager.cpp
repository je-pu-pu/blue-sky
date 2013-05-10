#include "GraphicsManager.h"
#include "FbxFileLoader.h"

#include "DrawingMesh.h"
#include "DrawingLine.h"

namespace blue_sky
{

/**
 * Žè•`‚«•—ƒƒbƒVƒ…‚ð“Ç‚Ýž‚Þ
 *
 * @param name –¼‘O
 * @return Žè•`‚«•—ƒƒbƒVƒ…A‚Ü‚½‚ÍŽ¸”sŽž‚É 0 ‚ð•Ô‚·
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
 * Žè•`‚«•—ƒ‰ƒCƒ“‚ð“Ç‚Ýž‚Þ
 *
 * @param name –¼‘O
 * @return Žè•`‚«•—ƒ‰ƒCƒ“A‚Ü‚½‚ÍŽ¸”sŽž‚É 0 ‚ð•Ô‚·
 */
DrawingLine* GraphicsManager::load_drawing_line( const char_t* name )
{
	DrawingLine* line = create_drawing_line();
	line->load_obj( ( string_t( "media/model/" ) + name + "-line.obj" ).c_str() );

	return line;
}

}