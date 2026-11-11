#include "ObjFileLoader.h"
#include <blue_sky/GameMain.h>
#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/graphics/Model.h>
#include <blue_sky/graphics/Mesh.h>
#include <common/exception.h>
#include <boost/algorithm/string/regex.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>

namespace blue_sky::graphics
{

/**
 * OBJ ファイルを読み込む
 *
 * usemtl の無い .obj は何もしないシェーダー（null）で読み込むので描かれない（今の素材はそれを前提にしている）。
 * マテリアルの無いモデルを描きたいときは、usemtl を書くか、最初の面の前に create_shader() で作ったシェーダーを割り当てる。
 *
 * 面の番号が範囲外・書き方が正しくない・頂点が 3 つ未満のときは、ファイル名と行番号を付けて例外を投げる
 * （以前は範囲外の番号でそのまま配列を読み、UV の無い "1//3" は法線の番号を UV の番号として読んでいた）。
 * UV が書かれていない頂点は UV ( 0, 0 ) にする。4 つ以上の頂点の面は扇形に三角形へ分ける
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

	int line_number = 0;

	/// 不完全な .obj のとき、どこが悪いかを付けて投げる
	auto fail = [&] ( const std::string& reason, const std::string& line )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( std::string( file_name ) + " の " + std::to_string( line_number ) + " 行目 : " + reason + " ( " + line + " )" );
	};

	while ( in.good() )
	{
		std::string line;
		std::getline( in, line );
		line_number++;

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
			std::vector< std::string > f_list;

			for ( std::string f; ss >> f; )
			{
				f_list.push_back( f );
			}

			if ( f_list.size() < 3 )
			{
				fail( "面の頂点が 3 つ未満", line );
			}

			// Z を反転したので、巻き順も逆にする
			std::reverse( f_list.begin(), f_list.end() );

			std::vector< Vertex > face;

			for ( const auto& f : f_list )
			{
				const auto fv = obj::parse_face_vertex( f );

				if ( ! fv )
				{
					fail( "面の頂点の書き方が正しくない", line );
				}

				if ( fv->position >= position_list.size() || ( fv->tex_coord && *fv->tex_coord >= tex_coord_list.size() ) || ( fv->normal && *fv->normal >= normal_list.size() ) )
				{
					fail( "面の番号が範囲外", line );
				}

				Vertex v;

				v.Position = position_list[ fv->position ];

				if ( fv->tex_coord )
				{
					v.TexCoord = tex_coord_list[ *fv->tex_coord ];
					v.TexCoord.y() = 1.f - v.TexCoord.y();
				}
				else
				{
					v.TexCoord = Mesh::TexCoord( 0.f, 0.f );
				}

				if ( fv->normal )
				{
					v.Normal = normal_list[ *fv->normal ];
				}

				face.push_back( v );
			}

			auto add_vertex = [&] ( const Vertex& v )
			{
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
			};

			// 三角形はそのまま、4 つ以上は扇形に分ける
			for ( size_t n = 1; n + 1 < face.size(); n++ )
			{
				add_vertex( face[ 0 ] );
				add_vertex( face[ n ] );
				add_vertex( face[ n + 1 ] );
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

namespace obj
{

std::optional< FaceVertex > parse_face_vertex( const std::string& text )
{
	std::vector< std::string > fields;
	std::string::size_type start = 0;

	while ( true )
	{
		const auto slash = text.find( '/', start );
		fields.push_back( text.substr( start, slash == std::string::npos ? std::string::npos : slash - start ) );

		if ( slash == std::string::npos )
		{
			break;
		}

		start = slash + 1;
	}

	if ( fields.size() > 3 )
	{
		return std::nullopt;
	}

	/// 1 始まりの番号を 0 始まりにする（空の欄は「書かれていない」）
	auto to_index = [] ( const std::string& s, bool& ok ) -> std::optional< size_t >
	{
		if ( s.empty() )
		{
			return std::nullopt;
		}

		if ( s.find_first_not_of( "0123456789" ) != std::string::npos || s.size() > 9 )
		{
			ok = false;
			return std::nullopt;
		}

		const auto n = std::stoul( s );

		if ( n == 0 )
		{
			ok = false;
			return std::nullopt;
		}

		return static_cast< size_t >( n - 1 );
	};

	bool ok = true;
	const auto position = to_index( fields[ 0 ], ok );

	if ( ! position )
	{
		return std::nullopt;
	}

	FaceVertex result;
	result.position = *position;

	if ( fields.size() > 1 )
	{
		result.tex_coord = to_index( fields[ 1 ], ok );
	}

	if ( fields.size() > 2 )
	{
		result.normal = to_index( fields[ 2 ], ok );
	}

	if ( ! ok )
	{
		return std::nullopt;
	}

	return result;
}

} // namespace obj

} // namespace blue_sky::graphics
