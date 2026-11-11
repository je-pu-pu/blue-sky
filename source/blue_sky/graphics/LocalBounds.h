#pragma once

#include <blue_sky/type.h>

#include <algorithm>
#include <cmath>

namespace blue_sky::graphics
{

/**
 * モデルの座標で頂点を囲む箱（見えない物を描かないため = カリングに使う）
 *
 * メッシュ・線を読むときに頂点から求めて覚える（頂点の一覧は GPU に送った後で消すので、その前に）
 */
struct LocalBounds
{
	Vector3	min;
	Vector3	max;
	bool	empty = true;	///< 頂点が 1 つも無い（求めていない）

	void add( const Vector3& p )
	{
		if ( empty )
		{
			min = p;
			max = p;
			empty = false;
			return;
		}

		min = Vector3( std::min( min.x(), p.x() ), std::min( min.y(), p.y() ), std::min( min.z(), p.z() ) );
		max = Vector3( std::max( max.x(), p.x() ), std::max( max.y(), p.y() ), std::max( max.z(), p.z() ) );
	}

	void add( const LocalBounds& b )
	{
		if ( b.empty )
		{
			return;
		}

		add( b.min );
		add( b.max );
	}

	/// 箱を囲む球の中心
	Vector3 center() const { return ( min + max ) * 0.5f; }

	/// 箱を囲む球の半径
	float_t radius() const { return ( max - min ).length() * 0.5f; }
};

} // namespace blue_sky::graphics
