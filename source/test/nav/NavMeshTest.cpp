#include "pch.h"

// NavMesh（Recast ベイク＋Detour 経路探索）の単体テスト。
#include <core/nav/NavMesh.h>

TEST( NavMeshTest, BuildFlatGroundAndFindPath )
{
	// 20x20 の平らな地面（2三角形）。法線が上向き(+Y)になる巻き順にする。
	const float verts[] = {
		-10.f, 0.f, -10.f,   //0
		 10.f, 0.f, -10.f,   //1
		 10.f, 0.f,  10.f,   //2
		-10.f, 0.f,  10.f,   //3
	};
	const int tris[] = { 0, 2, 1,  0, 3, 2 };

	core::nav::NavMesh nav;
	core::nav::NavMeshBuildConfig cfg;
	ASSERT_TRUE( nav.build( verts, 4, tris, 2, cfg ) );
	ASSERT_TRUE( nav.is_built() );

	std::vector< core::math::Vector > path;
	const bool ok = nav.find_path(
		core::math::Vector( -7.f, 0.f, -7.f ),
		core::math::Vector(  7.f, 0.f,  7.f ),
		path );

	EXPECT_TRUE( ok );
	ASSERT_GE( path.size(), 2u );

	// 経路の終点はゴール付近に来るはず（平地なのでほぼ直線）
	const auto& last = path.back();
	EXPECT_NEAR( last.x(), 7.f, 1.5f );
	EXPECT_NEAR( last.z(), 7.f, 1.5f );
}

TEST( NavMeshTest, FindPathFailsWhenNotBuilt )
{
	core::nav::NavMesh nav;
	std::vector< core::math::Vector > path;
	EXPECT_FALSE( nav.find_path( core::math::Vector( 0, 0, 0 ), core::math::Vector( 1, 0, 1 ), path ) );
}
