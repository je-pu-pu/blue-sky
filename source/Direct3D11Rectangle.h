#ifndef DIRECT_3D_11_RECTANGLE_H
#define DIRECT_3D_11_RECTANGLE_H

#include "Direct3D11Mesh.h"

/**
 * Direct3D 11 Rectangle
 *
 */
class Direct3D11Rectangle : public Direct3D11Mesh 
{
public:
	Direct3D11Rectangle( Direct3D11* );
	~Direct3D11Rectangle() { }

	bool has_animation() const { return false; }

}; // class Direct3D11Rectangle

#endif // DIRECT_3D_11_RECTANGLE_H
