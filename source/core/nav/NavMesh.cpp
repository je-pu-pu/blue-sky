#include "NavMesh.h"

#include <recastnavigation/Recast.h>
#include <recastnavigation/DetourNavMesh.h>
#include <recastnavigation/DetourNavMeshBuilder.h>
#include <recastnavigation/DetourNavMeshQuery.h>

#include <cmath>
#include <cstring>

namespace core::nav
{

namespace
{
	// このラッパで歩行可能ポリゴンに立てるフラグ（クエリで許可する）。
	constexpr unsigned short POLY_FLAG_WALK = 0x01;
}

NavMesh::NavMesh() = default;

NavMesh::~NavMesh()
{
	release();
}

void NavMesh::release()
{
	if ( nav_query_ ) { dtFreeNavMeshQuery( nav_query_ ); nav_query_ = nullptr; }
	if ( nav_mesh_ )  { dtFreeNavMesh( nav_mesh_ );        nav_mesh_  = nullptr; }
}

bool NavMesh::build( const float* verts, int nverts, const int* tris, int ntris, const NavMeshBuildConfig& c )
{
	release();

	if ( ! verts || ! tris || nverts <= 0 || ntris <= 0 )
	{
		return false;
	}

	// ジオメトリのバウンディングボックス
	float bmin[ 3 ], bmax[ 3 ];
	rcCalcBounds( verts, nverts, bmin, bmax );

	// Recast 設定
	rcConfig cfg;
	std::memset( & cfg, 0, sizeof( cfg ) );
	cfg.cs                     = c.cell_size;
	cfg.ch                     = c.cell_height;
	cfg.walkableSlopeAngle     = c.agent_max_slope_deg;
	cfg.walkableHeight         = (int) std::ceil( c.agent_height / cfg.ch );
	cfg.walkableClimb          = (int) std::floor( c.agent_max_climb / cfg.ch );
	cfg.walkableRadius         = (int) std::ceil( c.agent_radius / cfg.cs );
	cfg.maxEdgeLen             = (int) ( c.edge_max_len / cfg.cs );
	cfg.maxSimplificationError = c.edge_max_error;
	cfg.minRegionArea          = (int) rcSqr( c.region_min_size );
	cfg.mergeRegionArea        = (int) rcSqr( c.region_merge_size );
	cfg.maxVertsPerPoly        = (int) c.verts_per_poly;
	cfg.detailSampleDist       = c.detail_sample_dist < 0.9f ? 0.f : cfg.cs * c.detail_sample_dist;
	cfg.detailSampleMaxError   = cfg.ch * c.detail_sample_max_err;
	rcVcopy( cfg.bmin, bmin );
	rcVcopy( cfg.bmax, bmax );
	rcCalcGridSize( cfg.bmin, cfg.bmax, cfg.cs, & cfg.width, & cfg.height );

	rcContext ctx( false );

	// 1. ハイトフィールド
	rcHeightfield* solid = rcAllocHeightfield();
	if ( ! solid || ! rcCreateHeightfield( & ctx, * solid, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch ) )
	{
		rcFreeHeightField( solid );
		return false;
	}

	// 2. 歩行可能三角形をラスタライズ
	std::vector< unsigned char > areas( ntris, 0 );
	rcMarkWalkableTriangles( & ctx, cfg.walkableSlopeAngle, verts, nverts, tris, ntris, areas.data() );
	if ( ! rcRasterizeTriangles( & ctx, verts, nverts, tris, areas.data(), ntris, * solid, cfg.walkableClimb ) )
	{
		rcFreeHeightField( solid );
		return false;
	}

	// 3. フィルタ
	rcFilterLowHangingWalkableObstacles( & ctx, cfg.walkableClimb, * solid );
	rcFilterLedgeSpans( & ctx, cfg.walkableHeight, cfg.walkableClimb, * solid );
	rcFilterWalkableLowHeightSpans( & ctx, cfg.walkableHeight, * solid );

	// 4. コンパクトハイトフィールド
	rcCompactHeightfield* chf = rcAllocCompactHeightfield();
	if ( ! chf || ! rcBuildCompactHeightfield( & ctx, cfg.walkableHeight, cfg.walkableClimb, * solid, * chf ) )
	{
		rcFreeHeightField( solid );
		rcFreeCompactHeightfield( chf );
		return false;
	}
	rcFreeHeightField( solid );
	solid = nullptr;

	if ( ! rcErodeWalkableArea( & ctx, cfg.walkableRadius, * chf ) ||
	     ! rcBuildDistanceField( & ctx, * chf ) ||
	     ! rcBuildRegions( & ctx, * chf, 0, cfg.minRegionArea, cfg.mergeRegionArea ) )
	{
		rcFreeCompactHeightfield( chf );
		return false;
	}

	// 5. 輪郭
	rcContourSet* cset = rcAllocContourSet();
	if ( ! cset || ! rcBuildContours( & ctx, * chf, cfg.maxSimplificationError, cfg.maxEdgeLen, * cset ) )
	{
		rcFreeCompactHeightfield( chf );
		rcFreeContourSet( cset );
		return false;
	}

	// 6. ポリゴンメッシュ
	rcPolyMesh* pmesh = rcAllocPolyMesh();
	if ( ! pmesh || ! rcBuildPolyMesh( & ctx, * cset, cfg.maxVertsPerPoly, * pmesh ) )
	{
		rcFreeCompactHeightfield( chf );
		rcFreeContourSet( cset );
		rcFreePolyMesh( pmesh );
		return false;
	}

	// 7. 詳細メッシュ
	rcPolyMeshDetail* dmesh = rcAllocPolyMeshDetail();
	if ( ! dmesh || ! rcBuildPolyMeshDetail( & ctx, * pmesh, * chf, cfg.detailSampleDist, cfg.detailSampleMaxError, * dmesh ) )
	{
		rcFreeCompactHeightfield( chf );
		rcFreeContourSet( cset );
		rcFreePolyMesh( pmesh );
		rcFreePolyMeshDetail( dmesh );
		return false;
	}
	rcFreeCompactHeightfield( chf );
	rcFreeContourSet( cset );

	// 歩行可能ポリゴンにフラグを立てる（Detour のクエリで通れるように）
	for ( int i = 0; i < pmesh->npolys; ++i )
	{
		if ( pmesh->areas[ i ] == RC_WALKABLE_AREA )
		{
			pmesh->flags[ i ] = POLY_FLAG_WALK;
		}
	}

	// 8. Detour navmesh データを生成
	dtNavMeshCreateParams params;
	std::memset( & params, 0, sizeof( params ) );
	params.verts            = pmesh->verts;
	params.vertCount        = pmesh->nverts;
	params.polys            = pmesh->polys;
	params.polyAreas        = pmesh->areas;
	params.polyFlags        = pmesh->flags;
	params.polyCount        = pmesh->npolys;
	params.nvp              = pmesh->nvp;
	params.detailMeshes     = dmesh->meshes;
	params.detailVerts      = dmesh->verts;
	params.detailVertsCount = dmesh->nverts;
	params.detailTris       = dmesh->tris;
	params.detailTriCount   = dmesh->ntris;
	params.walkableHeight   = c.agent_height;
	params.walkableRadius   = c.agent_radius;
	params.walkableClimb    = c.agent_max_climb;

	// オフメッシュ接続（島同士を橋渡し）。Detour 用の平坦配列に詰める。
	// これらは dtCreateNavMeshData 呼び出しまで生存している必要がある（同一スコープ）。
	std::vector< float >          om_verts;   // 6 floats/link (start xyz, end xyz)
	std::vector< float >          om_rad;
	std::vector< unsigned char >  om_areas;
	std::vector< unsigned short > om_flags;
	std::vector< unsigned char >  om_dir;
	std::vector< unsigned int >   om_id;
	for ( size_t i = 0; i < off_mesh_links_.size(); ++i )
	{
		const OffMeshLink& lk = off_mesh_links_[ i ];
		om_verts.push_back( lk.start.x() ); om_verts.push_back( lk.start.y() ); om_verts.push_back( lk.start.z() );
		om_verts.push_back( lk.end.x() );   om_verts.push_back( lk.end.y() );   om_verts.push_back( lk.end.z() );
		om_rad.push_back( lk.radius );
		om_areas.push_back( RC_WALKABLE_AREA );
		om_flags.push_back( POLY_FLAG_WALK );
		om_dir.push_back( lk.bidirectional ? DT_OFFMESH_CON_BIDIR : 0 );
		om_id.push_back( static_cast< unsigned int >( 0x1000 + i ) );
	}
	if ( ! off_mesh_links_.empty() )
	{
		params.offMeshConVerts  = om_verts.data();
		params.offMeshConRad    = om_rad.data();
		params.offMeshConAreas  = om_areas.data();
		params.offMeshConFlags  = om_flags.data();
		params.offMeshConDir    = om_dir.data();
		params.offMeshConUserID = om_id.data();
		params.offMeshConCount  = static_cast< int >( off_mesh_links_.size() );
	}

	rcVcopy( params.bmin, pmesh->bmin );
	rcVcopy( params.bmax, pmesh->bmax );
	params.cs               = cfg.cs;
	params.ch               = cfg.ch;
	params.buildBvTree      = true;

	unsigned char* nav_data = nullptr;
	int nav_data_size = 0;
	const bool ok = dtCreateNavMeshData( & params, & nav_data, & nav_data_size );

	rcFreePolyMesh( pmesh );
	rcFreePolyMeshDetail( dmesh );

	if ( ! ok )
	{
		return false;
	}

	// 9. dtNavMesh / dtNavMeshQuery を初期化（single tile。データは navmesh が所有）
	nav_mesh_ = dtAllocNavMesh();
	if ( ! nav_mesh_ || dtStatusFailed( nav_mesh_->init( nav_data, nav_data_size, DT_TILE_FREE_DATA ) ) )
	{
		dtFree( nav_data );
		release();
		return false;
	}

	nav_query_ = dtAllocNavMeshQuery();
	if ( ! nav_query_ || dtStatusFailed( nav_query_->init( nav_mesh_, 2048 ) ) )
	{
		release();
		return false;
	}

	return true;
}

unsigned char NavMesh::off_mesh_connection_flag()
{
	return DT_STRAIGHTPATH_OFFMESH_CONNECTION;
}

bool NavMesh::find_path( const core::math::Vector& start, const core::math::Vector& end,
	std::vector< core::math::Vector >& out,
	std::vector< unsigned char >* out_flags ) const
{
	out.clear();
	if ( out_flags ) { out_flags->clear(); }

	if ( ! nav_mesh_ || ! nav_query_ )
	{
		return false;
	}

	dtQueryFilter filter;
	filter.setIncludeFlags( POLY_FLAG_WALK );
	filter.setExcludeFlags( 0 );

	const float sp[ 3 ] = { start.x(), start.y(), start.z() };
	const float ep[ 3 ] = { end.x(),   end.y(),   end.z() };
	const float extents[ 3 ] = { 2.f, 4.f, 2.f };  // 最近ポリゴン探索の許容範囲

	dtPolyRef start_ref = 0, end_ref = 0;
	float start_nearest[ 3 ], end_nearest[ 3 ];
	nav_query_->findNearestPoly( sp, extents, & filter, & start_ref, start_nearest );
	nav_query_->findNearestPoly( ep, extents, & filter, & end_ref,   end_nearest );

	if ( ! start_ref || ! end_ref )
	{
		return false; // 始点/終点が navmesh 上に見つからない
	}

	// ポリゴン経路
	static const int MAX_POLYS = 256;
	dtPolyRef path[ MAX_POLYS ];
	int path_count = 0;
	if ( dtStatusFailed( nav_query_->findPath( start_ref, end_ref, start_nearest, end_nearest, & filter, path, & path_count, MAX_POLYS ) )
		|| path_count == 0 )
	{
		return false;
	}

	// 折れ線（string-pulling）。フラグも取得（オフメッシュ接続の入口判定に使う）。
	float         straight[ MAX_POLYS * 3 ];
	unsigned char straight_flags[ MAX_POLYS ];
	int straight_count = 0;
	if ( dtStatusFailed( nav_query_->findStraightPath( start_nearest, end_nearest, path, path_count,
		straight, straight_flags, nullptr, & straight_count, MAX_POLYS ) ) )
	{
		return false;
	}

	out.reserve( straight_count );
	if ( out_flags ) { out_flags->reserve( straight_count ); }
	for ( int i = 0; i < straight_count; ++i )
	{
		out.emplace_back( straight[ i * 3 + 0 ], straight[ i * 3 + 1 ], straight[ i * 3 + 2 ] );
		if ( out_flags ) { out_flags->push_back( straight_flags[ i ] ); }
	}
	return ! out.empty();
}

bool NavMesh::sample_height( const core::math::Vector& pos, float& out_y ) const
{
	if ( ! nav_mesh_ || ! nav_query_ )
	{
		return false;
	}

	dtQueryFilter filter;
	filter.setIncludeFlags( POLY_FLAG_WALK );
	filter.setExcludeFlags( 0 );

	const float p[ 3 ] = { pos.x(), pos.y(), pos.z() };
	// XZ は狭く・縦はやや広く探して「今いる段（レベル）」の足元面を拾う。
	// 縦を広げすぎると地面の上にある屋上を誤って拾うので 2m に抑える。
	const float extents[ 3 ] = { 1.f, 2.f, 1.f };

	dtPolyRef ref = 0;
	float nearest[ 3 ];
	nav_query_->findNearestPoly( p, extents, & filter, & ref, nearest );
	if ( ! ref )
	{
		return false;
	}

	float h = 0.f;
	if ( dtStatusFailed( nav_query_->getPolyHeight( ref, nearest, & h ) ) )
	{
		out_y = nearest[ 1 ];   // フォールバック: 最近ポリゴン上の点の高さ
		return true;
	}
	out_y = h;
	return true;
}

void NavMesh::debug_get_poly_edges( std::vector< core::math::Vector >& out ) const
{
	out.clear();
	if ( ! nav_mesh_ )
	{
		return;
	}

	const dtNavMesh* nav = nav_mesh_;   // const 版 getTile()（public）を呼ぶため
	for ( int t = 0; t < nav->getMaxTiles(); ++t )
	{
		const dtMeshTile* tile = nav->getTile( t );
		if ( ! tile || ! tile->header )
		{
			continue;
		}

		for ( int i = 0; i < tile->header->polyCount; ++i )
		{
			const dtPoly& poly = tile->polys[ i ];
			if ( poly.getType() == DT_POLYTYPE_OFFMESH_CONNECTION )
			{
				continue;
			}

			const int nv = poly.vertCount;
			for ( int j = 0; j < nv; ++j )
			{
				const float* va = & tile->verts[ poly.verts[ j ] * 3 ];
				const float* vb = & tile->verts[ poly.verts[ ( j + 1 ) % nv ] * 3 ];
				out.emplace_back( va[ 0 ], va[ 1 ], va[ 2 ] );
				out.emplace_back( vb[ 0 ], vb[ 1 ], vb[ 2 ] );
			}
		}
	}
}

} // namespace core::nav
