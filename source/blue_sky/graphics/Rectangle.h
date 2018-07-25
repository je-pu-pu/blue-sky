#pragma once

#include <blue_sky/graphics/Mesh.h>

namespace blue_sky::graphics
{

/**
 * éläpå`
 *
 */
class Rectangle : public Mesh 
{
public:
	explicit Rectangle( Buffer* );
	virtual ~Rectangle() { }

}; // class Rectangle

} // namespace blue_sky::graphics
