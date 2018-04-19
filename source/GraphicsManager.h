#ifndef BLUE_SKY_GRAPHICS_MANAGER_H
#define BLUE_SKY_GRAPHICS_MANAGER_H

#include "ConstantBuffer.h"

#include <core/type.h>

#include <game/GraphicsManager.h>

#include <common/safe_ptr.h>
#include <common/auto_ptr.h>

class SkinningAnimationSet;
class FbxFileLoader;

namespace game
{

class Texture;

}

namespace blue_sky
{

class DrawingMesh;
class DrawingLine;

/**
 * グラフィック管理クラス
 *
 */
class GraphicsManager : public game::GraphicsManager
{
public:
	typedef game::Texture Texture;

private:
	common::auto_ptr< FbxFileLoader> fbx_file_loader_;

public:
	GraphicsManager();
	virtual ~GraphicsManager();

	virtual DrawingMesh* create_drawing_mesh() = 0;
	virtual DrawingLine* create_drawing_line() = 0;

	DrawingMesh* load_drawing_mesh( const char_t*, common::safe_ptr< SkinningAnimationSet >& );
	DrawingLine* load_drawing_line( const char_t* );
	
	virtual Texture* load_texture( const char_t*, const char_t* ) = 0;
	virtual Texture* get_texture( const char_t* ) = 0;
	
	/// @todo ちゃんと作る Font, size, 指定した Texture への描画
	virtual void draw_text( float_t, float_t, float_t, float_t, const char_t*, const Color& ) const = 0;
	virtual void draw_text_at_center( const char_t*, const Color& ) const = 0;

	virtual void unload_texture( const char_t* ) = 0;
	virtual void unload_texture_all() = 0;

	virtual GameConstantBuffer* get_game_render_data() const = 0;
	virtual FrameConstantBuffer* get_frame_render_data() const = 0;
	virtual FrameDrawingConstantBuffer* get_frame_drawing_render_data() const = 0;
	virtual ObjectConstantBuffer* get_shared_object_render_data() const = 0;

}; // class GraphicsManager

} // namespace blue_sky

#endif // BLUE_SKY_GRAPHICS_MANAGER_H
