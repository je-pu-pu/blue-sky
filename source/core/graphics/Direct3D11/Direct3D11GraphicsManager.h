#ifndef BLUE_SKY_DIRECT_3D_11_GRAPHICS_MANAGER_H
#define BLUE_SKY_DIRECT_3D_11_GRAPHICS_MANAGER_H

#include "GraphicsManager.h"
#include "ConstantBuffer.h"

#include <memory>

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
	
	std::unique_ptr< GameConstantBuffer >			game_render_data_;
	std::unique_ptr< FrameConstantBuffer >			frame_render_data_;
	std::unique_ptr< FrameDrawingConstantBuffer >	frame_drawing_render_data_;
	std::unique_ptr< ObjectConstantBuffer >			shared_object_render_data_;

public:
	Direct3D11GraphicsManager( Direct3D* );
	virtual ~Direct3D11GraphicsManager();

	Mesh* create_mesh();
	Line* create_line();

	DrawingMesh* create_drawing_mesh();
	DrawingLine* create_drawing_line();

	Texture* get_texture( const char_t* );

	GameConstantBuffer* get_game_render_data() const override { return game_render_data_.get(); }
	FrameConstantBuffer* get_frame_render_data() const override { return frame_render_data_.get(); }
	FrameDrawingConstantBuffer* get_frame_drawing_render_data() const override { return frame_drawing_render_data_.get(); }
	ObjectConstantBuffer* get_shared_object_render_data() const override { return shared_object_render_data_.get(); }

}; // class GraphicsManager

} // namespace game

#endif // BLUE_SKY_DIRECT_3D_11_GRAPHICS_MANAGER_H
