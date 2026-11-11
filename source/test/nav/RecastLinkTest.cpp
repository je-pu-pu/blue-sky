#include "pch.h"

// Recast/Detour の x64 静的リンク疎通スモークテスト（ヘッダ include ＋シンボル解決の確認）。
#include <recastnavigation/Recast.h>
#include <recastnavigation/DetourNavMesh.h>
#include <recastnavigation/DetourNavMeshQuery.h>

TEST( RecastLinkTest, AllocateAndFree )
{
	// Detour: navmesh とクエリを確保・解放できる（＝リンク成功）
	dtNavMesh* nav = dtAllocNavMesh();
	ASSERT_NE( nullptr, nav );

	dtNavMeshQuery* query = dtAllocNavMeshQuery();
	ASSERT_NE( nullptr, query );

	dtFreeNavMeshQuery( query );
	dtFreeNavMesh( nav );

	// Recast: コンテキストを構築できる（ヘッダとリンクの両方を確認）
	rcContext ctx;
	ctx.resetTimers();
	SUCCEED();
}
