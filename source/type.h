#ifndef BLUE_SKY_TYPE_H
#define BLUE_SKY_TYPE_H

#include <type/type.h>

class Direct3D11Vector;
class Direct3D11Matrix;

typedef Direct3D11Vector Vector;
typedef Direct3D11Matrix Matrix;

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#endif // BLUE_SKY_TYPE_H
