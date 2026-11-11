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

namespace
{
	// EnemyAiTestScene と同じ「地面＋屋上のあるビル」を三角形で組む。
	// ビルは footprint x[-10,-6]×z[10,14], top y=4 のソリッドボックス（屋上が歩行面の島になる）。
	void build_ground_and_building( std::vector< float >& verts, std::vector< int >& tris )
	{
		auto add_vertex = [&]( float x, float y, float z ) -> int
		{
			const int idx = static_cast< int >( verts.size() / 3 );
			verts.push_back( x ); verts.push_back( y ); verts.push_back( z );
			return idx;
		};
		auto add_tri = [&]( int a, int b, int c ) { tris.push_back( a ); tris.push_back( b ); tris.push_back( c ); };

		// 地面 -25..25
		const int v0 = add_vertex( -25.f, 0.f, -25.f );
		const int v1 = add_vertex(  25.f, 0.f, -25.f );
		const int v2 = add_vertex(  25.f, 0.f,  25.f );
		const int v3 = add_vertex( -25.f, 0.f,  25.f );
		add_tri( v0, v2, v1 ); add_tri( v0, v3, v2 );

		// ビル（ソリッドボックス）
		const float x0 = -10.f, x1 = -6.f, z0 = 10.f, z1 = 14.f, y0 = 0.05f, y1 = 4.f;
		const int b0 = add_vertex( x0, y0, z0 ), b1 = add_vertex( x1, y0, z0 ), b2 = add_vertex( x1, y0, z1 ), b3 = add_vertex( x0, y0, z1 );
		const int t0 = add_vertex( x0, y1, z0 ), t1 = add_vertex( x1, y1, z0 ), t2 = add_vertex( x1, y1, z1 ), t3 = add_vertex( x0, y1, z1 );
		add_tri( b0, t0, t1 ); add_tri( b0, t1, b1 );
		add_tri( b1, t1, t2 ); add_tri( b1, t2, b2 );
		add_tri( b2, t2, t3 ); add_tri( b2, t3, b3 );
		add_tri( b3, t3, t0 ); add_tri( b3, t0, b0 );
		add_tri( t0, t2, t1 ); add_tri( t0, t3, t2 );
	}
}

namespace
{
	// EnemyAiTestScene の実際の障害物配置（全6ビル）＋地面を組む。
	void build_full_scene( std::vector< float >& verts, std::vector< int >& tris )
	{
		auto add_vertex = [&]( float x, float y, float z ) -> int
		{
			const int idx = static_cast< int >( verts.size() / 3 );
			verts.push_back( x ); verts.push_back( y ); verts.push_back( z );
			return idx;
		};
		auto add_tri = [&]( int a, int b, int c ) { tris.push_back( a ); tris.push_back( b ); tris.push_back( c ); };

		const int v0 = add_vertex( -25.f, 0.f, -25.f ), v1 = add_vertex( 25.f, 0.f, -25.f );
		const int v2 = add_vertex(  25.f, 0.f,  25.f ), v3 = add_vertex( -25.f, 0.f, 25.f );
		add_tri( v0, v2, v1 ); add_tri( v0, v3, v2 );

		auto add_box = [&]( float cx, float cz, float hx, float hz, float top )
		{
			const float x0 = cx - hx, x1 = cx + hx, z0 = cz - hz, z1 = cz + hz, y0 = 0.05f, y1 = top;
			const int b0 = add_vertex( x0, y0, z0 ), b1 = add_vertex( x1, y0, z0 ), b2 = add_vertex( x1, y0, z1 ), b3 = add_vertex( x0, y0, z1 );
			const int t0 = add_vertex( x0, y1, z0 ), t1 = add_vertex( x1, y1, z0 ), t2 = add_vertex( x1, y1, z1 ), t3 = add_vertex( x0, y1, z1 );
			add_tri( b0, t0, t1 ); add_tri( b0, t1, b1 ); add_tri( b1, t1, t2 ); add_tri( b1, t2, b2 );
			add_tri( b2, t2, t3 ); add_tri( b2, t3, b3 ); add_tri( b3, t3, t0 ); add_tri( b3, t0, b0 );
			add_tri( t0, t2, t1 ); add_tri( t0, t3, t2 );
		};
		add_box(  8.f,  4.f, 2.0f, 2.0f, 5.f  );
		add_box(  8.f, 12.f, 2.0f, 2.0f, 8.f  );
		add_box( -8.f,  4.f, 2.0f, 2.0f, 6.f  );   // 階段の南にあり旧リンク入口(z4.5)を塞ぐビル
		add_box( -8.f, 12.f, 2.0f, 2.0f, 4.f  );   // 屋上へ登る対象（roof y4, z[10,14]）
		add_box(  0.f, 17.f, 3.0f, 2.0f, 3.f  );
		add_box(  4.f, -2.f, 1.5f, 1.5f, 2.f  );
	}

	bool link_reaches_roof( const core::nav::OffMeshLink& lk, const core::math::Vector& from )
	{
		std::vector< float > verts; std::vector< int > tris;
		build_full_scene( verts, tris );
		core::nav::NavMesh nav;
		nav.set_off_mesh_links( { lk } );
		core::nav::NavMeshBuildConfig cfg; cfg.agent_radius = 0.5f;
		EXPECT_TRUE( nav.build( verts.data(), (int)( verts.size() / 3 ), tris.data(), (int)( tris.size() / 3 ), cfg ) );
		std::vector< core::math::Vector > path;
		const bool ok = nav.find_path( from, core::math::Vector( -8, 4, 12 ), path );
		const float last_y = path.empty() ? -999.f : path.back().y();
		std::cout << "  link entry z=" << lk.start.z() << " ok=" << ok << " last_y=" << last_y << "\n";
		return ok && last_y > 3.f;
	}
}

// 実配置（全6ビル）で、旧リンク(南 z4.5＝ビルの中)は屋上へ繋がらず、北 z18 のリンクは繋がること。
TEST( NavMeshTest, FullSceneLinkMustAvoidBuildings )
{
	// 旧: 入口(-8,0,4.5) は (-8,z4) ビル(z2-6)の中 → 連結しない
	const bool old_ok = link_reaches_roof(
		{ core::math::Vector( -8, 0, 4.5f ), core::math::Vector( -8, 4, 11.f ), 0.8f, true },
		core::math::Vector( -8, 0, 8 ) );
	EXPECT_FALSE( old_ok ) << "旧リンク入口はビルの中なので屋上へ繋がらないはず";

	// 新: 入口(-8,0,18) は北側の空き地、出口(-8,4,13) は屋上（エロージョン内側）→ 連結する
	const bool new_ok = link_reaches_roof(
		{ core::math::Vector( -8, 0, 18.f ), core::math::Vector( -8, 4, 13.f ), 0.8f, true },
		core::math::Vector( -8, 0, 20 ) );
	EXPECT_TRUE( new_ok ) << "北側リンクは屋上へ繋がるはず";
}

namespace
{
	// EnemyAiTestScene の合成ビル構成（ビルA/B＝段違い2箱、遮蔽ビル）を組む。
	void build_composite_scene( std::vector< float >& verts, std::vector< int >& tris )
	{
		auto add_vertex = [&]( float x, float y, float z ) -> int
		{
			const int idx = static_cast< int >( verts.size() / 3 );
			verts.push_back( x ); verts.push_back( y ); verts.push_back( z );
			return idx;
		};
		auto add_tri = [&]( int a, int b, int c ) { tris.push_back( a ); tris.push_back( b ); tris.push_back( c ); };

		const int v0 = add_vertex( -25.f, 0.f, -25.f ), v1 = add_vertex( 25.f, 0.f, -25.f );
		const int v2 = add_vertex(  25.f, 0.f,  25.f ), v3 = add_vertex( -25.f, 0.f, 25.f );
		add_tri( v0, v2, v1 ); add_tri( v0, v3, v2 );

		auto add_box = [&]( float cx, float cz, float hx, float hz, float top )
		{
			const float x0 = cx - hx, x1 = cx + hx, z0 = cz - hz, z1 = cz + hz, y0 = 0.05f, y1 = top;
			const int b0 = add_vertex( x0, y0, z0 ), b1 = add_vertex( x1, y0, z0 ), b2 = add_vertex( x1, y0, z1 ), b3 = add_vertex( x0, y0, z1 );
			const int t0 = add_vertex( x0, y1, z0 ), t1 = add_vertex( x1, y1, z0 ), t2 = add_vertex( x1, y1, z1 ), t3 = add_vertex( x0, y1, z1 );
			add_tri( b0, t0, t1 ); add_tri( b0, t1, b1 ); add_tri( b1, t1, t2 ); add_tri( b1, t2, b2 );
			add_tri( b2, t2, t3 ); add_tri( b2, t3, b3 ); add_tri( b3, t3, t0 ); add_tri( b3, t0, b0 );
			add_tri( t0, t2, t1 ); add_tri( t0, t3, t2 );
		};
		add_box( -11.f, 12.f, 2.f, 2.f, 3.f );  // A-2F roof y3
		add_box(  -5.f, 12.f, 2.f, 2.f, 5.f );  // A-3F roof y5
		add_box(   5.f, 12.f, 2.f, 2.f, 5.f );  // B-3F roof y5（西）
		add_box(  11.f, 12.f, 2.f, 2.f, 3.f );  // B-2F roof y3（東）
		add_box(  -8.f,  4.f, 2.f, 2.f, 6.f );  // 遮蔽
		add_box(   8.f,  4.f, 2.f, 2.f, 5.f );
		add_box(   0.f, 17.f, 3.f, 2.f, 3.f );
		add_box(   4.f, -2.f, 1.5f, 1.5f, 2.f );
	}

	std::vector< core::nav::OffMeshLink > composite_links()
	{
		// add_stairs / add_bridge が inset=1 で作る端点（見た目の縁の外側＝屋上の内側）と一致させる。
		return {
			{ core::math::Vector( -11, 0, 19 ), core::math::Vector( -11, 3, 13 ), 0.8f, true }, // 地上→A-2F
			{ core::math::Vector( -10, 3, 12 ), core::math::Vector(  -6, 5, 12 ), 0.8f, true }, // A-2F→A-3F（隙間）
			{ core::math::Vector(  -4, 5, 12 ), core::math::Vector(   4, 5, 12 ), 0.9f, true }, // A-3F→B-3F 橋（隙間）
			{ core::math::Vector(  10, 3, 12 ), core::math::Vector(   6, 5, 12 ), 0.8f, true }, // B-3F→B-2F（隙間）
			{ core::math::Vector(  11, 0, 19 ), core::math::Vector(  11, 3, 13 ), 0.8f, true }, // 地上→B-2F（東・ループ）
			{ core::math::Vector(  -8, 0, -5 ), core::math::Vector(  -8, 6,  3 ), 0.8f, true }, // 地上→h6 見張り台
		};
	}

	float composite_path_top( const core::math::Vector& from, const core::math::Vector& to )
	{
		std::vector< float > verts; std::vector< int > tris;
		build_composite_scene( verts, tris );
		core::nav::NavMesh nav;
		nav.set_off_mesh_links( composite_links() );
		core::nav::NavMeshBuildConfig cfg; cfg.agent_radius = 0.5f;
		EXPECT_TRUE( nav.build( verts.data(), (int)( verts.size() / 3 ), tris.data(), (int)( tris.size() / 3 ), cfg ) );
		std::vector< core::math::Vector > path;
		const bool ok = nav.find_path( from, to, path );
		const float y = ( ok && ! path.empty() ) ? path.back().y() : -999.f;
		std::cout << "  from(" << from.x() << "," << from.z() << ") to(" << to.x() << "," << to.z()
		          << ") ok=" << ok << " last_y=" << y << "\n";
		return y;
	}
}

// 合成ビル＋コネクタ（階段2・橋1・階層間1）で、地上→屋上・屋上間の橋・全連鎖が繋がること。
TEST( NavMeshTest, CompositeStageConnectorsChain )
{
	// 地上 → A-3F（地上→A-2F→A-3F の2連鎖）
	EXPECT_NEAR( composite_path_top( core::math::Vector( -11, 0, 20 ), core::math::Vector( -5, 5, 12 ) ), 5.f, 1.0f );
	// A-3F → B-3F（橋）
	EXPECT_NEAR( composite_path_top( core::math::Vector( -5, 5, 12 ), core::math::Vector( 5, 5, 12 ) ), 5.f, 1.0f );
	// 地上 → B-2F（4リンク全連鎖）
	EXPECT_NEAR( composite_path_top( core::math::Vector( -11, 0, 20 ), core::math::Vector( 11, 3, 12 ) ), 3.f, 1.0f );

	// --- 複数ルート ---
	// 東の入口から直接 B-2F（A を経由しない別ルート）
	EXPECT_NEAR( composite_path_top( core::math::Vector( 11, 0, 20 ), core::math::Vector( 11, 3, 12 ) ), 3.f, 1.0f );
	// 東から登って橋で A-3F へ（ループの反対回り）
	EXPECT_NEAR( composite_path_top( core::math::Vector( 11, 0, 20 ), core::math::Vector( -5, 5, 12 ) ), 5.f, 1.0f );
	// h6 見張り台へ地上から
	EXPECT_NEAR( composite_path_top( core::math::Vector( -8, 0, -6 ), core::math::Vector( -8, 6, 4 ) ), 6.f, 1.0f );
}

// 橋（水平オフメッシュ接続）を渡る経路に、オフメッシュ入口フラグが立つこと。
// PatrolSystem はこのフラグで「リンク横断中(on_link)」を判定して 3D 直進する。
// フラグが立たないと橋上で歩行面が無く縁停止して動けなくなる。
TEST( NavMeshTest, BridgePathHasOffMeshFlag )
{
	std::vector< float > verts; std::vector< int > tris;
	build_composite_scene( verts, tris );
	core::nav::NavMesh nav;
	nav.set_off_mesh_links( composite_links() );
	core::nav::NavMeshBuildConfig cfg; cfg.agent_radius = 0.5f;
	ASSERT_TRUE( nav.build( verts.data(), (int)( verts.size() / 3 ), tris.data(), (int)( tris.size() / 3 ), cfg ) );

	std::vector< core::math::Vector > path;
	std::vector< unsigned char >      flags;
	const bool ok = nav.find_path( core::math::Vector( -5, 5, 12 ), core::math::Vector( 5, 5, 12 ), path, & flags );
	ASSERT_TRUE( ok );
	ASSERT_EQ( path.size(), flags.size() );

	bool has_offmesh = false;
	std::cout << "[bridge path]\n";
	for ( std::size_t i = 0; i < path.size(); ++i )
	{
		const bool om = ( flags[ i ] & core::nav::NavMesh::off_mesh_connection_flag() ) != 0;
		has_offmesh = has_offmesh || om;
		std::cout << "  (" << path[i].x() << "," << path[i].y() << "," << path[i].z() << ") flag=" << (int)flags[i]
		          << ( om ? " <OFFMESH>" : "" ) << "\n";
	}
	EXPECT_TRUE( has_offmesh ) << "橋の経路にオフメッシュ入口フラグが無い → on_link 判定できず橋上で停止する";
}

// オフメッシュ接続で地上→屋上（別々の歩行面の島）が連結され、経路が屋上へ届くこと。
// 傾斜面を navmesh に焼くとエロージョン/二重床で分断されるため、EnemyAiTestScene は
// オフメッシュ接続で地上↔屋上を橋渡ししている。その回帰テスト。
TEST( NavMeshTest, OffMeshLinkConnectsGroundToRooftop )
{
	std::vector< float > verts;
	std::vector< int >   tris;
	build_ground_and_building( verts, tris );

	core::nav::NavMesh nav;

	// 北側の階段下の地面 ↔ 屋上 を橋渡し
	std::vector< core::nav::OffMeshLink > links;
	links.push_back( { core::math::Vector( -8.f, 0.f, 18.f ), core::math::Vector( -8.f, 4.f, 13.f ), 0.8f, true } );
	nav.set_off_mesh_links( links );

	core::nav::NavMeshBuildConfig cfg;
	cfg.agent_radius = 0.5f;
	ASSERT_TRUE( nav.build( verts.data(), (int)( verts.size() / 3 ), tris.data(), (int)( tris.size() / 3 ), cfg ) );

	std::vector< core::math::Vector > path;
	const bool ok = nav.find_path(
		core::math::Vector( -8.f, 0.f, 20.f ),   // 北の地上
		core::math::Vector( -8.f, 4.f, 12.f ),   // 屋上
		path );

	EXPECT_TRUE( ok );
	ASSERT_GE( path.size(), 2u );
	EXPECT_NEAR( path.back().y(), 4.f, 1.0f ) << "オフメッシュ接続でも屋上へ到達できていない";
	EXPECT_NEAR( path.back().z(), 12.f, 2.0f );
}

// オフメッシュ接続が無ければ地上→屋上は連結しない（島が分かれたまま）ことの確認。
TEST( NavMeshTest, WithoutLinkGroundAndRooftopAreSeparate )
{
	std::vector< float > verts;
	std::vector< int >   tris;
	build_ground_and_building( verts, tris );

	core::nav::NavMesh nav;
	core::nav::NavMeshBuildConfig cfg;
	cfg.agent_radius = 0.5f;
	ASSERT_TRUE( nav.build( verts.data(), (int)( verts.size() / 3 ), tris.data(), (int)( tris.size() / 3 ), cfg ) );

	std::vector< core::math::Vector > path;
	nav.find_path( core::math::Vector( -8, 0, 3 ), core::math::Vector( -8, 4, 12 ), path );

	// 屋上（y4）には届かない（地上側の島に留まる）
	if ( ! path.empty() )
	{
		EXPECT_LT( path.back().y(), 2.0f );
	}
}

// sample_height が屋上（別の島）の高さを拾えること（PatrolSystem の面スナップ用）。
TEST( NavMeshTest, SampleHeightOnRooftop )
{
	std::vector< float > verts;
	std::vector< int >   tris;
	build_ground_and_building( verts, tris );

	core::nav::NavMesh nav;
	core::nav::NavMeshBuildConfig cfg;
	cfg.agent_radius = 0.5f;
	ASSERT_TRUE( nav.build( verts.data(), (int)( verts.size() / 3 ), tris.data(), (int)( tris.size() / 3 ), cfg ) );

	float y = -999.f;
	const bool ok = nav.sample_height( core::math::Vector( -8.f, 4.f, 12.f ), y );
	EXPECT_TRUE( ok );
	EXPECT_NEAR( y, 4.f, 0.5f );

	// 地上も拾える
	float yg = -999.f;
	EXPECT_TRUE( nav.sample_height( core::math::Vector( -8.f, 0.f, 3.f ), yg ) );
	EXPECT_NEAR( yg, 0.f, 0.5f );
}
