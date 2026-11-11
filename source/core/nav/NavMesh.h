#pragma once

#include <core/math/Vector.h>
#include <vector>

class dtNavMesh;
class dtNavMeshQuery;

namespace core::nav
{

/// オフメッシュ接続（階段/ジャンプ等で navmesh の別々の島を橋渡しする Detour のリンク）。
/// start/end はそれぞれ既存の歩行面の近くに置くこと（Detour が最寄りポリゴンへ接続する）。
struct OffMeshLink
{
	core::math::Vector start;             ///< 一方の端点（例: 階段下の地面）
	core::math::Vector end;               ///< もう一方の端点（例: 屋上の縁）
	float              radius = 0.6f;     ///< 接続半径 (m)
	bool               bidirectional = true; ///< 双方向に通れるか
};

/// navmesh ベイクのパラメータ（Recast）。
struct NavMeshBuildConfig
{
	float cell_size              = 0.3f;   ///< ボクセルの水平サイズ (m)
	float cell_height            = 0.2f;   ///< ボクセルの高さ (m)
	float agent_height           = 2.0f;   ///< エージェントの高さ (m)
	float agent_radius           = 0.4f;   ///< エージェントの半径 (m)
	float agent_max_climb        = 0.4f;   ///< 登れる段差 (m)
	float agent_max_slope_deg    = 45.0f;  ///< 歩ける最大傾斜 (度)
	float region_min_size        = 8.0f;   ///< 最小リージョン（セル数の平方根）
	float region_merge_size      = 20.0f;  ///< リージョン統合サイズ
	float edge_max_len           = 12.0f;  ///< 輪郭エッジ最大長 (m)
	float edge_max_error         = 1.3f;   ///< 輪郭の最大誤差
	float verts_per_poly         = 6.0f;   ///< ポリゴンあたり最大頂点数
	float detail_sample_dist     = 6.0f;   ///< 詳細メッシュのサンプル間隔
	float detail_sample_max_err  = 1.0f;   ///< 詳細メッシュの最大誤差
};

/**
 * Recast でベイクした navmesh を Detour で経路探索するラッパ（単一タイル・ソロメッシュ）。
 *
 * build() で三角形ジオメトリから navmesh を焼き、find_path() で歩行面上の経路を得る。
 * 敵AIのパトロール/追跡経路に使う。
 */
class NavMesh
{
public:
	NavMesh();
	~NavMesh();

	NavMesh( const NavMesh& ) = delete;
	NavMesh& operator = ( const NavMesh& ) = delete;

	/**
	 * 三角形ジオメトリから navmesh をベイクする。
	 * @param verts  頂点配列 (x,y,z の連続, 長さ 3*nverts)
	 * @param nverts 頂点数
	 * @param tris   三角形インデックス (長さ 3*ntris)
	 * @param ntris  三角形数
	 * @return 成功で true
	 */
	bool build( const float* verts, int nverts, const int* tris, int ntris, const NavMeshBuildConfig& cfg );

	/// 次回の build() で焼き込むオフメッシュ接続を設定する（build より前に呼ぶ）。
	void set_off_mesh_links( const std::vector< OffMeshLink >& links ) { off_mesh_links_ = links; }

	/**
	 * start→end の経路（歩行面上の折れ線）を out に得る。
	 * @param out_flags 非 null なら各点の Detour straight-path フラグを返す
	 *                  （DT_STRAIGHTPATH_OFFMESH_CONNECTION 等。オフメッシュ区間の判定に使う）
	 * @return 経路が得られたら true（out は始点側から並ぶ）
	 */
	bool find_path( const core::math::Vector& start, const core::math::Vector& end,
		std::vector< core::math::Vector >& out,
		std::vector< unsigned char >* out_flags = nullptr ) const;

	/// 経路点フラグに立つ「この点はオフメッシュ接続の入口」を表すビット。
	static unsigned char off_mesh_connection_flag();

	/**
	 * pos の XZ 位置で足元の歩行面の高さを得る（縦方向は pos.y 近傍を探す）。
	 * 敵をキネマティックに面へ張り付かせる（空中を滑らせない）用途。
	 * @return 歩行面が見つかれば true（out_y に面の高さ）
	 */
	bool sample_height( const core::math::Vector& pos, float& out_y ) const;

	bool is_built() const { return nav_mesh_ != nullptr; }

	/// デバッグ描画用に navmesh のポリゴン辺を線分ペア（a0,b0, a1,b1, ...）で得る。
	void debug_get_poly_edges( std::vector< core::math::Vector >& out_lines ) const;

private:
	void release();

	dtNavMesh*      nav_mesh_  = nullptr;
	dtNavMeshQuery* nav_query_ = nullptr;

	std::vector< OffMeshLink > off_mesh_links_;   ///< build() で焼き込むオフメッシュ接続

}; // class NavMesh

} // namespace core::nav
