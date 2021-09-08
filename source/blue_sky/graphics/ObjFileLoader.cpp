#include "ObjFileLoader.h"
#include <GameMain.h>
#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/graphics/Model.h>
#include <blue_sky/graphics/Mesh.h>
#include <common/exception.h>
#include <boost/algorithm/string/regex.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <sstream>
#include <list>

namespace blue_sky::graphics
{

/**
 * OBJ ファイルを読み込む
 *
 * @todo マテリアル・テクスチャの指定のないモデルも読み込めるようにする ( plane.obj, teapot.obj など )
 * @todo 不完全な .obj を読み込んだ時にエラーを出力する
 * 
 * @param file_name OBJ ファイル名
 * @return ファイルの読み込みに成功した場合は true を、失敗した場合は false を返す
 */
bool ObjFileLoader::load( const char_t* file_name )
{
	// common::log( "log/debug.log", string_t( "loading obj : " ) + file_name );

	std::ifstream in( file_name );

	if ( ! in.good() )
	{
		return false;
	}

	get_model()->set_mesh( create_mesh() );

	VertexIndexMap vertex_index_map;

	Mesh::PositionList position_list;
	Mesh::NormalList normal_list;
	Mesh::TexCoordList tex_coord_list;

	Mesh::VertexGroup* vertex_group = get_model()->get_mesh()->create_vertex_group();
	
	Shader* shader = get_null_shader();
	get_model()->set_shader_at( 0, shader );

	int shader_index = 0;

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
			Mesh::Position v;

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
			Mesh::TexCoord uv;

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

				Mesh::PositionList::size_type position_index;
				Mesh::TexCoordList::size_type tex_coord_index;
				Mesh::NormalList::size_type normal_index;

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
					Mesh::Index vertex_index = static_cast< Mesh::Index >( get_model()->get_mesh()->get_vertex_count() );

					get_model()->get_mesh()->add_vertex( v );
					vertex_group->add_index( vertex_index );

					vertex_index_map[ v ] = vertex_index;
				}
			}
		}
		else if ( command == "texture" )
		{
			ss >> texture_name;
			shader->set_texture_at( 0, load_texture( texture_name.c_str() ) );
		}
		else if ( command == "usemtl" )
		{
			if ( ! vertex_group->empty() )
			{
				vertex_group = get_model()->get_mesh()->create_vertex_group();
			}

			shader = create_shader();
			get_model()->set_shader_at( shader_index, shader );
			shader_index++;
			
			ss >> texture_name;

			boost::algorithm::replace_all_regex( texture_name, boost::regex( "(.*)_(.*)" ), std::string( "$2" ) );

			if ( ! texture_name.empty() )
			{
				try
				{
					shader->set_texture_at( 0, load_texture( texture_name.c_str() ) );
				}
				catch ( ... )
				{
					COMMON_THROW_EXCEPTION_MESSAGE( texture_name );
				}
			}
		}
	}

	// !!!
	if ( texture_name.empty() )
	{
		try
		{
			boost::filesystem::path path( file_name );
			auto* texture = load_texture( path.stem().string().c_str() );

			if ( texture )
			{
				if ( ! shader )
				{
					shader = create_shader();
				}

				shader->set_texture_at( 0, texture );
			}
		}
		catch ( ... )
		{

		}
	}

	return true;
}

} // namespace blue_sky::graphics
