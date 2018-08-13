#include "Vector.h"

namespace direct_x_math
{

const Vector Vector::Zero    = Vector(  0.f,  0.f,  0.f );
const Vector Vector::Left    = Vector( -1.f,  0.f,  0.f );
const Vector Vector::Right   = Vector(  1.f,  0.f,  0.f );
const Vector Vector::Up      = Vector(  0.f,  1.f,  0.f );
const Vector Vector::Down    = Vector(  0.f, -1.f,  0.f );
const Vector Vector::Forward = Vector(  0.f,  0.f,  1.f );
const Vector Vector::Back    = Vector(  0.f,  0.f, -1.f );

}; // namespace direct_x_math
