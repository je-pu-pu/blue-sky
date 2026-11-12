#pragma once

#include <core/math/DirectXMath/Transform.h>
#include <core/math/DirectXMath/Matrix.h>
#include <core/math/DirectXMath/Quaternion.h>
#include <core/math/DirectXMath/Color.h>
#include <core/math/DirectXMath/Vector.h>
#include <core/math/DirectXMath/Vector3.h>
#include <core/math/DirectXMath/Vector2.h>

#include <type/type.h>

#include <core/ecs/type.h>   // core::ecs::ComponentTypeId / SystemTypeId 等（定義はこちらに一本化）

namespace core
{
	namespace graphics
	{
		/// ピクセルフォーマット（エンジン独自の型。定義は core/graphics/PixelFormat.h）
		enum class PixelFormat;
	}

	using Vector2			= math::direct_x_math::Vector2;
	using Vector3			= math::direct_x_math::Vector3;
	using Vector			= math::direct_x_math::Vector;
	using Matrix			= math::direct_x_math::Matrix;
	using Color				= math::direct_x_math::Color;

	using Transform			= math::direct_x_math::Transform;
	using Quaternion		= math::direct_x_math::Quaternion;

} // namespace core
