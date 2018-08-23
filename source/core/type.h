#pragma once

#include <core/math/DirectXMath/Vector2.h>
#include <core/math/DirectXMath/Vector3.h>
#include <core/math/DirectXMath/Vector.h>
#include <core/math/DirectXMath/Matrix.h>
#include <core/math/DirectXMath/Color.h>
#include <core/math/Bullet/Quaternion.h>
#include <core/math/Bullet/Transform.h>
#include <type/type.h>

namespace core
{
	/*
	namespace direct_x_math
	{
		class Vector2;
		class Vector3;
		class Vector;
		class Matrix;
		class Color;
	}

	namespace bullet
	{
		class Quaternion;
		class Transform;
	}
	*/

	typedef direct_x_math::Vector2			Vector2;
	typedef direct_x_math::Vector3			Vector3;
	typedef direct_x_math::Vector			Vector;
	typedef direct_x_math::Matrix			Matrix;
	typedef direct_x_math::Color			Color;

	typedef core::bullet::Quaternion		Quaternion;
	typedef core::bullet::Transform			Transform;

} // namespace core
