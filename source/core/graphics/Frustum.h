#pragma once

#include <core/type.h>

#include <DirectXMath.h>

#include <cmath>

namespace core::graphics
{

/**
 * 視錐台（描く範囲を囲む 6 つの面）。見えない物を描かないため（カリング）に、球が範囲に掛かるかを調べる
 *
 * view * projection（行ベクトルに右から掛ける D3D の決まり。深さは 0〜1）から面を取り出す。
 * 透視（カメラ）でも平行（影のカスケード）でも同じに使える。
 * 調べるのは「球が 6 つの面のどれかの完全に外側にあるか」だけなので、外と言ったら必ず描かれない（隅の近くでは外でも中と言うことがある）
 */
class Frustum
{
public:
	/// 面 a x + b y + c z + d >= 0 が内側（a, b, c は長さ 1 にしてある）
	struct Plane
	{
		float_t a = 0.f, b = 0.f, c = 0.f, d = 0.f;

		float_t distance( float_t x, float_t y, float_t z ) const { return a * x + b * y + c * z + d; }
	};

	static constexpr int PlaneCount = 6;

private:
	Plane planes_[ PlaneCount ];	///< 左・右・下・上・手前・奥（既定は全部 0 で、どの球も内側）

	static Plane make_plane( float_t a, float_t b, float_t c, float_t d )
	{
		const float_t length = std::sqrt( a * a + b * b + c * c );

		if ( length <= 0.f )
		{
			return Plane{ 0.f, 0.f, 0.f, 0.f };	// 面にならない（行列が壊れている）ときは何も外さない
		}

		return Plane{ a / length, b / length, c / length, d / length };
	}

public:
	/// 何も外さない視錐台
	Frustum() = default;

	/**
	 * view * projection から作る
	 *
	 * 切り取り空間 ( x, y, z, w ) の内側は -w <= x <= w・-w <= y <= w・0 <= z <= w。
	 * 世界の点 p に対して x = p・列 0 なので、例えば左の面は 列 3 + 列 0 >= 0 になる
	 */
	static Frustum from_view_projection( const Matrix& view_projection )
	{
		DirectX::XMFLOAT4X4 m;
		DirectX::XMStoreFloat4x4( & m, view_projection );

		// 列 3 ± 列 j（列 j = ( m[ 0 ][ j ], m[ 1 ][ j ], m[ 2 ][ j ], m[ 3 ][ j ] )）
		auto w_with = [ & ] ( int j, float_t sign )
		{
			return make_plane(
				m.m[ 0 ][ 3 ] + sign * m.m[ 0 ][ j ],
				m.m[ 1 ][ 3 ] + sign * m.m[ 1 ][ j ],
				m.m[ 2 ][ 3 ] + sign * m.m[ 2 ][ j ],
				m.m[ 3 ][ 3 ] + sign * m.m[ 3 ][ j ] );
		};

		Frustum f;
		f.planes_[ 0 ] = w_with( 0,  1.f );		// 左 : w + x >= 0
		f.planes_[ 1 ] = w_with( 0, -1.f );		// 右 : w - x >= 0
		f.planes_[ 2 ] = w_with( 1,  1.f );		// 下 : w + y >= 0
		f.planes_[ 3 ] = w_with( 1, -1.f );		// 上 : w - y >= 0
		f.planes_[ 4 ] = make_plane( m.m[ 0 ][ 2 ], m.m[ 1 ][ 2 ], m.m[ 2 ][ 2 ], m.m[ 3 ][ 2 ] );	// 手前 : z >= 0
		f.planes_[ 5 ] = w_with( 2, -1.f );		// 奥 : w - z >= 0

		return f;
	}

	const Plane& get_plane( int n ) const { return planes_[ n ]; }

	/// 中心 ( x, y, z )・半径 radius の球が、範囲に掛かるか（どれかの面の完全に外側なら false）
	bool intersects_sphere( float_t x, float_t y, float_t z, float_t radius ) const
	{
		for ( const auto& p : planes_ )
		{
			if ( p.distance( x, y, z ) < -radius )
			{
				return false;
			}
		}

		return true;
	}

	bool intersects_sphere( const Vector& center, float_t radius ) const
	{
		return intersects_sphere( center.x(), center.y(), center.z(), radius );
	}
};

} // namespace core::graphics
