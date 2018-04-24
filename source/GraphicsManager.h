#ifndef BLUE_SKY_GRAPHICS_MANAGER_H
#define BLUE_SKY_GRAPHICS_MANAGER_H

#include "ConstantBuffer.h"

#include <core/type.h>

#include <game/GraphicsManager.h>

#include <common/safe_ptr.h>
#include <common/auto_ptr.h>

#include <vector>
#include <functional>

class Vector3;
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

class ActiveObject;
class ActiveObjectManager;

/**
 * グラフィック管理クラス
 *
 */
class GraphicsManager : public game::GraphicsManager
{
public:
	typedef game::Texture Texture;

private:
	std::vector< Texture* >		paper_texture_list_;
	Texture*					paper_texture_ = 0;

	common::auto_ptr< FbxFileLoader> fbx_file_loader_;
	bool is_debug_axis_enabled_ = true;

protected:
	void render_debug_axis_for_bones( const ActiveObject* ) const;
	virtual void render_debug_axis_model() const = 0;

public:
	GraphicsManager();
	virtual ~GraphicsManager();

	virtual void update() = 0;

	virtual DrawingMesh* create_drawing_mesh() = 0;
	virtual DrawingLine* create_drawing_line() = 0;

	DrawingMesh* load_drawing_mesh( const char_t*, common::safe_ptr< SkinningAnimationSet >& );
	DrawingLine* load_drawing_line( const char_t* );
	
	virtual Texture* load_texture( const char_t*, const char_t* ) = 0;
	virtual Texture* get_texture( const char_t* ) = 0;

	void load_paper_textures();

	virtual void unload_texture( const char_t* ) = 0;
	virtual void unload_texture_all() = 0;

	void set_paper_texture_type( int_t );
	void bind_paper_texture() const;

	virtual void set_sky_box( const char_t* ) = 0;
	virtual void unset_sky_box() = 0;
	virtual bool is_sky_box_set() const = 0;

	virtual void set_ground( const char_t* ) = 0;
	virtual void unset_ground() = 0;
	// virtual bool is_ground_set() const = 0;

	virtual void set_ambient_color( const Color& ) = 0;
	virtual void set_shadow_color( const Color& ) = 0;
	virtual void set_shadow_paper_color( const Color& ) = 0;
	virtual void set_drawing_accent( float_t ) = 0;
	virtual void set_drawing_line_type( int_t ) = 0;

	virtual void set_eye_position( const Vector3& ) = 0;

	virtual GameConstantBuffer* get_game_render_data() const = 0;
	virtual FrameConstantBuffer* get_frame_render_data() const = 0;
	virtual FrameDrawingConstantBuffer* get_frame_drawing_render_data() const = 0;
	virtual ObjectConstantBuffer* get_shared_object_render_data() const = 0;

	virtual void set_input_layout( const char_t* ) const = 0;

	virtual void setup_rendering() const = 0;
	virtual void render_technique( const char_t*, const std::function< void () >& ) const = 0;
	virtual void render_background() const = 0;
	virtual void render_active_objects( const ActiveObjectManager* ) const;

	/// @todo 整理する
	virtual void set_fade_color( const Color& ) = 0;

	virtual void start_fade_in( float_t speed ) = 0;
	virtual void start_fade_out( float_t speed ) = 0;

	virtual void fade_in( float_t speed = 1.f ) = 0;
	virtual void fade_out( float_t speed = 1.f ) = 0;

	virtual void render_fader() const = 0;

	/// @todo ちゃんと作る Font, size, 指定した Texture への描画
	virtual void draw_text( float_t, float_t, float_t, float_t, const char_t*, const Color& ) const = 0;
	virtual void draw_text_at_center( const char_t*, const Color& ) const = 0;

	void set_debug_axis_enabled( bool e ) { is_debug_axis_enabled_ = e; }

	void render_debug_axis( const ActiveObjectManager* ) const;

	virtual void clear_debug_bullet() const = 0;
	virtual void render_debug_bullet() const = 0;
	
	// void update_frame_render_data( const Camera*, const Vector3& ) const;

}; // class GraphicsManager

} // namespace blue_sky

#endif // BLUE_SKY_GRAPHICS_MANAGER_H
