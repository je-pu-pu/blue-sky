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
	namespace graphics
	{
		namespace direct_3d_11
		{
			template< int > class ConstantBuffer;
			enum class PixelFormat;
		}

		using direct_3d_11::PixelFormat;
	}

	using graphics::direct_3d_11::ConstantBuffer;

	using direct_x_math::Vector2;
	using direct_x_math::Vector3;
	using direct_x_math::Vector;
	using direct_x_math::Matrix;
	using direct_x_math::Color;

	using bullet::Quaternion;
	using bullet::Transform;

	namespace ecs
	{
		using ComponentTypeId = uint_t;
		using SystemTypeId = uint_t;
	}

} // namespace core
