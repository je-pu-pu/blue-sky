#pragma once

#include <core/math/DirectXMath/Transform.h>
#include <core/math/DirectXMath/Matrix.h>
#include <core/math/DirectXMath/Quaternion.h>
#include <core/math/DirectXMath/Color.h>
#include <core/math/DirectXMath/Vector.h>
#include <core/math/DirectXMath/Vector3.h>
#include <core/math/DirectXMath/Vector2.h>

#include <type/type.h>

#include <cstddef>

namespace core
{
	namespace graphics
	{
		namespace direct_3d_11
		{
			template< int > class ConstantBuffer;
			enum class PixelFormat;
		}

		using PixelFormat	= direct_3d_11::PixelFormat;
	}

	template< int Slot >
	using ConstantBuffer	= graphics::direct_3d_11::ConstantBuffer< Slot >;

	using Vector2			= math::direct_x_math::Vector2;
	using Vector3			= math::direct_x_math::Vector3;
	using Vector			= math::direct_x_math::Vector;
	using Matrix			= math::direct_x_math::Matrix;
	using Color				= math::direct_x_math::Color;

	using Transform			= math::direct_x_math::Transform;
	using Quaternion		= math::direct_x_math::Quaternion;

	namespace ecs
	{
		// 型 ID は Component::get_type_id() が返す std::size_t（x64 で 64bit）と一致させる。
		// uint_t(32bit) だと x64 で切り捨てが起き、System の型一致判定が壊れる。
		using ComponentTypeId = std::size_t;
		using SystemTypeId = std::size_t;
	}

} // namespace core
