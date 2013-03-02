#ifndef BLUE_SKY_DRAWING_MODEL_H
#define BLUE_SKY_DRAWING_MODEL_H

#include "type.h"
#include <common/auto_ptr.h>

class Direct3D11Mesh;

namespace blue_sky
{

using common::auto_ptr;

// class DrawingMesh;
class DrawingLine;

/**
 * ��`�����f��
 *
 */
class DrawingModel
{
public:
	typedef Direct3D11Mesh		DrawingMesh;

protected:
	auto_ptr< DrawingMesh >		mesh_;		// ��
	auto_ptr< DrawingLine >		line_;		// ��

public:
	DrawingModel();
	~DrawingModel();

	void load( const char_t* );

	DrawingMesh* get_mesh() const { return mesh_.get(); }
	DrawingLine* get_line() const { return line_.get(); }

}; // class DrawingModel

} // namespace blue_sky

#endif // BLUE_SKY_DRAWING_MODEL_H
