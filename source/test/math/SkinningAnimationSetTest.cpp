#include "pch.h"

#include <core/animation/SkinningAnimationSet.h>

namespace
{

/// 点を行列で移した位置
core::Vector transform_point( const core::Matrix& m, const core::Vector& p )
{
	return p * m;
}

} // namespace

TEST( SkinningAnimationSetTest, ボーンの数を決めるとオフセット行列は単位行列になる )
{
	core::SkinningAnimationSet set;
	set.set_bone_count( 3 );

	ASSERT_EQ( 3u, set.get_bone_count() );

	const auto p = transform_point( set.get_bone_offset_matrix_by_bone_index( 2 ), core::Vector( 1.f, 2.f, 3.f, 1.f ) );
	EXPECT_FLOAT_EQ( 1.f, p.x() );
	EXPECT_FLOAT_EQ( 2.f, p.y() );
	EXPECT_FLOAT_EQ( 3.f, p.z() );
}

TEST( SkinningAnimationSetTest, コピーしてもオフセット行列は別々に持つ )
{
	// 以前は new した行列のポインタを持っていて、コピーすると同じ行列を二重に解放していた
	core::SkinningAnimationSet a;
	a.set_bone_count( 1 );

	{
		core::SkinningAnimationSet b = a;
		b.get_bone_offset_matrix_by_bone_index( 0 ).set_translation( 5.f, 0.f, 0.f );

		EXPECT_FLOAT_EQ( 5.f, transform_point( b.get_bone_offset_matrix_by_bone_index( 0 ), core::Vector( 0.f, 0.f, 0.f, 1.f ) ).x() );
	}

	EXPECT_FLOAT_EQ( 0.f, transform_point( a.get_bone_offset_matrix_by_bone_index( 0 ), core::Vector( 0.f, 0.f, 0.f, 1.f ) ).x() );
}
