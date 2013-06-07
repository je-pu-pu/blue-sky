#include "DrawingModel.h"
#include "DrawingMesh.h"
#include "DrawingLine.h"

#include "AnimationPlayer.h"

#include "GraphicsManager.h"

#include "GameMain.h"

namespace blue_sky
{

DrawingModel::DrawingModel()
{

}

DrawingModel::~DrawingModel()
{

}

void DrawingModel::load( const char_t* name )
{
	mesh_ = GameMain::get_instance()->get_graphics_manager()->load_drawing_mesh( name, skinning_animation_set_ );
	line_ = GameMain::get_instance()->get_graphics_manager()->load_drawing_line( name );
}

bool DrawingModel::has_animation() const
{
	return skinning_animation_set_;
}

bool DrawingModel::is_skin_mesh() const
{
	return skinning_animation_set_;
}

/**
 * アニメーション再生を作成する
 *
 */
AnimationPlayer* DrawingModel::create_animation_player() const
{
	if ( ! skinning_animation_set_ )
	{
		return 0;
	}

	return new AnimationPlayer( skinning_animation_set_.get() );
}

} // namespace blue_sky