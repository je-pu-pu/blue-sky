#include "Mesh.h"

namespace blue_sky::graphics
{

/**
 * 新しい頂点グループを作成する
 *
 * @return 作成された頂点グループ
 */
Mesh::VertexGroup* Mesh::create_vertex_group()
{
	vertex_group_list_.push_back( std::make_unique< VertexGroup >() );

	return vertex_group_list_.back().get();
}

/**
 * 頂点グループの一覧の中から指定したインデックスの頂点グループを取得する
 *
 * @param index 頂点グループのインデックス
 * @return 頂点グループ or 0
 */
Mesh::VertexGroup* Mesh::get_vertex_group_at( uint_t index )
{
	while ( index >= vertex_group_list_.size() )
	{
		create_vertex_group();
	}

	return vertex_group_list_[ index ].get();
}

/**
 * 最適化する
 *
 *
 */
void Mesh::optimize()
{
	
}

/**
 * システムメモリ上の頂点情報をクリアする
 *
 */
void Mesh::clear_vertex_list()
{
	vertex_list_.clear();
}

/**
 * システムメモリ上のウエイト情報をクリアする
 *
 */
void Mesh::clear_vertex_weight_list()
{
	vertex_weight_list_.clear();
}

/**
 * システムメモリ上のインデックス情報をクリアする
 *
 */
void Mesh::clear_vertex_group_list()
{
	vertex_group_list_.clear();
}

/**
 * 頂点の Z 値を反転する
 *
 */
void Mesh::invert_vertex_z()
{
	for ( auto& v : vertex_list_ )
	{
		v.Position.z() = -v.Position.z();
		v.Normal.z() = -v.Normal.z();
	}
}

/**
 * ポリゴンの裏表を反転する
 *
 */
void Mesh::flip_polygon()
{
	for ( auto& vertex_group : vertex_group_list_ )
	{
		vertex_group->flip_polygon();
	}
}

} // namespace blue_sky::graphics
