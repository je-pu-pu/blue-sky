#ifndef BLUE_SKY_MEMORY_H
#define BLUE_SKY_MEMORY_H

#define _CRTDBG_MAP_ALLOC

#include <crtdbg.h>

#ifdef _DEBUG
#define new ::new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#endif

#endif // BLUE_SKY_MEMORY_H
