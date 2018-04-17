#ifndef BLUE_SKY_DIRECT_3D_11_GRAPHICS_MANAGER_H
#define BLUE_SKY_DIRECT_3D_11_GRAPHICS_MANAGER_H

#include "GraphicsManager.h"

class Direct3D11;

namespace blue_sky
{

/**
 * Direct3D 11 グラフィック管理クラス
 *
 */
class Direct3D11GraphicsManager : public GraphicsManager
{
public:
	typedef Direct3D11 Direct3D;

private:
	Direct3D* direct_3d_;

public:
	Direct3D11GraphicsManager( Direct3D* );
	virtual ~Direct3D11GraphicsManager();

	Mesh* create_mesh();
	Line* create_line();

	DrawingMesh* create_drawing_mesh();
	DrawingLine* create_drawing_line();

	Texture* get_texture( const char_t* );

}; // class GraphicsManager

} // namespace game

#endif // BLUE_SKY_DIRECT_3D_11_GRAPHICS_MANAGER_H
