#ifndef BLUE_SKY_DRAWING_MODEL_H
#define BLUE_SKY_DRAWING_MODEL_H

#include "type.h"
#include <common/auto_ptr.h>
#include <common/safe_ptr.h>

class SkinningAnimationSet;
class AnimationPlayer;

namespace blue_sky
{

class DrawingMesh;
class DrawingLine;

/**
 * ��`�������f��
 *
 */
class DrawingModel
{
public:

private:
	common::auto_ptr< DrawingMesh >				mesh_;							///< ��`�������b�V��
	common::auto_ptr< DrawingLine >				line_;							///< ��`�������C��
	
	common::safe_ptr< SkinningAnimationSet >	skinning_animation_set_;		///< �A�j���[�V����

public:
	DrawingModel();
	~DrawingModel();

	void load( const char_t* );

	DrawingMesh* get_mesh() const { return mesh_.get(); }
	DrawingLine* get_line() const { return line_.get(); }

	bool has_animation() const;
	bool is_skin_mesh() const;

	AnimationPlayer* create_animation_player() const;

}; // class DrawingModel

} // namespace blue_sky

#endif // BLUE_SKY_DRAWING_MODEL_H
