#pragma once

#include <blue_sky/graphics/Mesh.h>

namespace blue_sky::graphics
{

/**
 * �l�p�`
 *
 */
class Rectangle : public Mesh 
{
public:
	explicit Rectangle( Buffer* );
	virtual ~Rectangle() override { }

}; // class Rectangle

} // namespace blue_sky::graphics
