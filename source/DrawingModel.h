#ifndef BLUE_SKY_DRAWING_MODEL_H
#define BLUE_SKY_DRAWING_MODEL_H

#include "type.h"
#include <common/auto_ptr.h>

class Direct3D11Mesh;

namespace blue_sky
{

class DrawingMesh;
class DrawingLine;

/**
 * 手描き風モデル
 *
 */
class DrawingModel
{
public:

private:
	common::auto_ptr< DrawingMesh >		mesh_;		///< 手描き風メッシュ
	common::auto_ptr< DrawingLine >		line_;		///< 手描き風ライン

public:
	DrawingModel();
	~DrawingModel();

	void load( const char_t* );

	DrawingMesh* get_mesh() const { return mesh_.get(); }
	DrawingLine* get_line() const { return line_.get(); }

}; // class DrawingModel

} // namespace blue_sky

#endif // BLUE_SKY_DRAWING_MODEL_H
