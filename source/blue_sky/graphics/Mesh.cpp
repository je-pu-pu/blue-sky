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

#if 0
/**
 * OBJ ファイルを読み込む
 *
 * @todo Loader として分離する
 *
 * @param file_name OBJ ファイル名
 * @return ファイルの読み込みに成功した場合は true を、失敗した場合は false を返す
 */
bool Direct3D11Mesh::load_obj( const char_t* file_name )
{
	// common::log( "log/debug.log", string_t( "loading obj : " ) + file_name );

	std::ifstream in( file_name );

	if ( ! in.good() )
	{
		return false;
	}

	typedef std::map< Vertex, Index > VertexIndexMap;
	VertexIndexMap vertex_index_map;

	PositionList position_list;
	NormalList normal_list;
	TexCoordList tex_coord_list;

	VertexGroup* vertex_group = create_vertex_group();
	Material* material = nullptr;

	std::string texture_name;

	while ( in.good() )
	{
		std::string line;		
		std::getline( in, line );

		std::stringstream ss;
		std::string command;
		
		ss << line;
		ss >> command;
		
		if ( command == "v" )
		{
			Position v;

			ss >> v.x() >> v.y() >> v.z();

			v.z() = -v.z();

			position_list.push_back( v );
		}
		else if ( command == "vn" )
		{
			Vector3 n;

			ss >> n.x() >> n.y() >> n.z();

			n.z() = -n.z();

			normal_list.push_back( n );
		}
		else if ( command == "vt" )
		{
			TexCoord uv;

			ss >> uv.x() >> uv.y();

			tex_coord_list.push_back( uv );
		}
		else if ( command == "f" )
		{
			std::list< std::string > f_list;

			while ( ss.good() )
			{
				std::string f;

				ss >> f;

				f_list.push_back( f );
			}

			if ( f_list.size() >= 3 )
			{
				f_list.reverse();
			}
			

			for ( auto fi = f_list.begin(); fi != f_list.end(); ++fi )
			{
				std::string& f = *fi;

				std::replace( f.begin(), f.end(), '/', ' ' );
				
				std::stringstream fss;
				fss << f;

				PositionList::size_type position_index;
				TexCoordList::size_type tex_coord_index;
				NormalList::size_type normal_index;

				fss >> position_index >> tex_coord_index;

				// 1 origin -> 0 origin
				position_index--;
				tex_coord_index--;

				Vertex v;
				
				v.Position = position_list[ position_index ];
				v.TexCoord = tex_coord_list[ tex_coord_index ];
				v.TexCoord.y() = 1.f - v.TexCoord.y();

				if ( ! fss.eof() )
				{
					fss >> normal_index;

					normal_index--;

					v.Normal = normal_list[ normal_index ];
				}

				VertexIndexMap::iterator i = vertex_index_map.find( v );

				if ( i != vertex_index_map.end() )
				{
					vertex_group->add_index( i->second );
				}
				else
				{
					Index vertex_index = static_cast< Index >( vertex_list_.size() );

					vertex_list_.push_back( v );
					vertex_group->add_index( vertex_index );

					vertex_index_map[ v ] = vertex_index;
				}
			}
		}
		else if ( command == "texture" )
		{
			ss >> texture_name;
			material->set_texture( load_texture_by_texture_name( texture_name.c_str() ) );
		}
		else if ( command == "usemtl" )
		{
			if ( ! vertex_group->empty() )
			{
				vertex_group = create_vertex_group();
			}

			material = create_material();
			
			ss >> texture_name;

			boost::algorithm::replace_all_regex( texture_name, boost::regex( "(.*)_(.*)" ), std::string( "$2" ) );

			if ( ! texture_name.empty() )
			{
				try
				{
					material->set_texture( load_texture_by_texture_name( texture_name.c_str() ) );
				}
				catch ( ... )
				{
					COMMON_THROW_EXCEPTION_MESSAGE( texture_name );
				}
			}
		}
	}

	optimize();

	create_vertex_buffer();
	create_index_buffer();

	clear_vertex_list();
	clear_vertex_group_list();
	clear_skinning_info_list();

	// !!!
	if ( texture_name.empty() )
	{
		try
		{
			boost::filesystem::path path( file_name );
			material->set_texture( load_texture_by_texture_name( path.stem().string().c_str() ) );
		}
		catch ( ... )
		{

		}
	}

	return true;
}

/**
 * FBX ファイルを読み込む
 *
 * @param file_name FBX ファイル名
 * @param loader FBX ファイルローダー
 * @return ファイルの読み込みに成功した場合は true を、失敗した場合は false を返す
 */
bool Direct3D11Mesh::load_fbx( const char_t* file_name, FbxFileLoader* loader )
{
	if ( ! loader->load( this, file_name ) )
	{
		return false;
	}

	optimize();

	create_vertex_buffer();
	create_index_buffer();

	clear_vertex_list();
	clear_vertex_group_list();
	clear_skinning_info_list();

	return true;
}

/**
 * FBX ファイルを読み込む
 *
 * @param file_name FBX ファイル名
 * @param loader FBX ファイルローダー
 * @param skinning_animation_set 同時に読み込むスキニングアニメーション情報を格納するポインタ
 * @return ファイルの読み込みに成功した場合は true を、失敗した場合は false を返す
 */
bool Direct3D11Mesh::load_fbx( const char_t* file_name, FbxFileLoader* loader, common::safe_ptr< SkinningAnimationSet >& skinning_animation_set )
{
	// common::log( "log/debug.log", string_t( "loading fbx : " ) + file_name );

	if ( ! loader->load( this, file_name ) )
	{
		return false;
	}

	skinning_animation_set = skinning_animation_set_.get();

	optimize();

	create_vertex_buffer();
	create_index_buffer();

	clear_vertex_list();
	clear_vertex_group_list();
	clear_skinning_info_list();

	return true;
}
#endif

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

#if 0
/**
 * 指定したテクスチャ名に対応するテクスチャファイルパスを取得する
 *
 * @param texture_name テクスチャ名
 * @return テクスチャファイルパス
 */
string_t Direct3D11Mesh::get_texture_file_path_by_texture_name( const char_t* texture_name ) const
{
	string_t texture_file_path = texture_name;

	boost::filesystem::path path( texture_file_path );
	
	if ( ! path.has_extension() )
	{
		texture_file_path += ".png";
	}

	if ( ! path.has_parent_path() )
	{
		texture_file_path = string_t( "media/model/" ) + texture_file_path;
	}

	return texture_file_path;
}

/**
 *  指定したテクスチャ名に対応するテクスチャファイルを読み込む
 *
 * @todo どこか別の場所に移動して整理する
 *
 * @param texture_name テクスチャ名
 * @return テクスチャ
 */
game::Texture* Direct3D11Mesh::load_texture_by_texture_name( const char_t* texture_name ) const
{
	auto texture_file_path = get_texture_file_path_by_texture_name( texture_name );
	return direct_3d_->getTextureManager()->load( texture_file_path.c_str(), texture_file_path.c_str() );
}
#endif

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
