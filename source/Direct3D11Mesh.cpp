#include "Direct3D11Mesh.h"
#include "Direct3D11Material.h"
#include "Direct3D11TextureManager.h"
#include "Direct3D11.h"
#include "DirectX.h"

#include "FbxFileLoader.h"

#include <common/exception.h>
#include <common/math.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string_regex.hpp>

#include <map>

#include <fstream>
#include <sstream>

/**
 * �R���X�g���N�^
 *
 * @param direct_3d Direct3D
 */
Direct3D11Mesh::Direct3D11Mesh( Direct3D11* direct_3d )
	: direct_3d_( direct_3d )
{

}

/**
 * �f�X�g���N�^
 *
 */
Direct3D11Mesh::~Direct3D11Mesh()
{
	for ( MaterialList::iterator i = material_list_.begin(); i != material_list_.end(); ++i )
	{
		delete *i;
	}

	for ( VertexBufferList::iterator i = vertex_buffer_list_.begin(); i != vertex_buffer_list_.end(); ++i )
	{
		DIRECT_X_RELEASE( *i );
	}
}

/**
 * �V�����}�e���A�����쐬����
 *
 * @return �쐬���ꂽ�}�e���A��
 */
Direct3D11Mesh::Material* Direct3D11Mesh::create_material()
{
	return new Material( direct_3d_ );
}

/**
 * OBJ �t�@�C����ǂݍ���
 *
 * @param file_name OBJ �t�@�C����
 * @return �t�@�C���̓ǂݍ��݂ɐ��������ꍇ�� true ���A���s�����ꍇ�� false ��Ԃ�
 */
bool Direct3D11Mesh::load_obj( const char_t* file_name )
{
	std::ifstream in( file_name );

	if ( ! in.good() )
	{
		return false;
	}

	typedef std::map< Vertex, Material::Index > VertexIndexMap;
	VertexIndexMap vertex_index_map;

	PositionList position_list;
	NormalList normal_list;
	TexCoordList tex_coord_list;

	material_list_.push_back( new Material( direct_3d_ ) );
	Material* material = material_list_[ material_list_.size() - 1 ];

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

			ss >> v.x >> v.y >> v.z;

			v.z = -v.z;

			position_list.push_back( v );
		}
		else if ( command == "vn" )
		{
			Vector3 n;

			ss >> n.x >> n.y >> n.z;

			normal_list.push_back( n );
		}
		else if ( command == "vt" )
		{
			TexCoord uv;

			ss >> uv.x >> uv.y;

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
				v.TexCoord.y = 1.f - v.TexCoord.y;

				if ( ! fss.eof() )
				{
					fss >> normal_index;

					normal_index--;

					v.Normal = normal_list[ normal_index ];
				}

				VertexIndexMap::iterator i = vertex_index_map.find( v );

				if ( i != vertex_index_map.end() )
				{
					material->get_index_list().push_back( i->second );
				}
				else
				{
					Material::Index vertex_index = vertex_list_.size();

					vertex_list_.push_back( v );
					material->get_index_list().push_back( vertex_index );

					vertex_index_map[ v ] = vertex_index;
				}
			}
		}
		else if ( command == "texture" )
		{
			ss >> texture_name;

			material->load_texture( get_texture_file_name_by_texture_name( texture_name.c_str() ).c_str() );
		}
		else if ( command == "usemtl" )
		{
			if ( ! material->get_index_list().empty() )
			{
				material->create_index_buffer();

				material_list_.push_back( new Material( direct_3d_ ) );
				material = material_list_[ material_list_.size() - 1 ];

				vertex_index_map.clear();
			}

			
			ss >> texture_name;

			boost::algorithm::replace_all_regex( texture_name, boost::regex( "(.*)_(.*)" ), std::string( "$2" ) );

			if ( ! texture_name.empty() )
			{
				string_t texture_file_name = get_texture_file_name_by_texture_name( texture_name.c_str() );
				
				if ( ! boost::regex_search( texture_file_name, boost::regex( "\\." ) ) )
				{
					texture_file_name += ".png";
				}

				try
				{
					material->load_texture( texture_file_name.c_str() );
				}
				catch ( ... )
				{
					COMMON_THROW_EXCEPTION_MESSAGE( texture_file_name );
				}
			}
		}
	}

	optimize();

	create_vertex_buffer();
	create_index_buffer( material );

	// !!!
	if ( texture_name.empty() )
	{
		boost::filesystem::path path( file_name );
		const string_t texture_file_name = std::string( "media/model/" ) + path.stem().string() + ".png";

		try
		{
			material->load_texture( get_texture_file_name_by_texture_name( texture_file_name.c_str() ).c_str() );
		}
		catch ( ... )
		{

		}
	}

	// test
	std::reverse( material_list_.begin(), material_list_.end() );

	return true;
}

/**
 * FBX �t�@�C����ǂݍ���
 *
 * @param file_name FBX �t�@�C����
 * @return �t�@�C���̓ǂݍ��݂ɐ��������ꍇ�� true ���A���s�����ꍇ�� false ��Ԃ�
 */
bool Direct3D11Mesh::load_fbx( const char_t* file_name )
{
	FbxFileLoader loader( this );

	if ( ! loader.load( file_name ) )
	{
		return false;
	}

	optimize();

	create_vertex_buffer();
	create_index_buffer();

	return true;
}

/**
 * �œK������
 *
 *
 */
void Direct3D11Mesh::optimize()
{
	struct VertexInfo
	{
		Vertex			vertex;
		SkinningInfo*	skinning_info;
		
		VertexInfo()
			: skinning_info( 0 )
		{

		}

		/*
		bool operator < ( const VertexInfo& v )
		{
			if ( vertex < v.vertex ) return true;
			if ( vertex > v.vertex ) return false;

			if ( ! skinning_info ) return true;
			if ( ! v.skinning_info ) return false;

			if ( *skinning_info < *v.skinning_info ) return true;
			
			return false;
		}
		*/
	};

	/*
	std::map< VertexInfo, Index > index_map;
	
	for ( VertexList::size_type n = 0; n < vertex_list_.size(); ++n )
	{
		VertexInfo v;
		v.vertex = vertex_list_[ n ];
		v.skinning_info = & skinning_info_list_[ n ];

		index_map.insert( std::make_pair( v, n ) );
	}
	*/

	// Index index;
}

/**
 * ���_�o�b�t�@���쐬����
 *
 */
void Direct3D11Mesh::create_vertex_buffer()
{
	if ( vertex_list_.empty() )
	{
		return;
	}

	{
		vertex_buffer_list_.push_back( 0 );

		D3D11_BUFFER_DESC buffer_desc = { 0 };

		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		buffer_desc.ByteWidth = sizeof( Vertex ) * vertex_list_.size();
    
		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = & vertex_list_[ 0 ];
	
		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, & data, & vertex_buffer_list_[ 0 ] ) );
	}

	if ( ! skinning_info_list_.empty() )
	{
		vertex_buffer_list_.push_back( 0 );

		D3D11_BUFFER_DESC buffer_desc = { 0 };

		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		buffer_desc.ByteWidth = sizeof( SkinningInfo ) * skinning_info_list_.size();
    
		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = & skinning_info_list_[ 0 ];
	
		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, & data, & vertex_buffer_list_[ 1 ] ) );
	}
}

/**
 * �C���f�b�N�X�o�b�t�@���쐬����
 *
 */
void Direct3D11Mesh::create_index_buffer()
{
	for ( auto i = get_material_list().begin(); i != get_material_list().end(); ++i )
	{
		create_index_buffer( *i );
	}
}

/**
 * �w�肵���}�e���A���ɑ΂���C���f�b�N�X�o�b�t�@���쐬����
 *
 * @param material �}�e���A��
 */
void Direct3D11Mesh::create_index_buffer( Material* material )
{
	material->create_index_buffer();
}

/**
 * �w�肵���e�N�X�`�����ɑΉ�����e�N�X�`���t�@�C�������擾����
 *
 * @param texture_name �e�N�X�`����
 * @return �e�N�X�`���t�@�C����
 */
string_t Direct3D11Mesh::get_texture_file_name_by_texture_name( const char_t* texture_name ) const
{
	/// !!!
	if ( ! boost::filesystem::path( texture_name ).has_parent_path() )
	{
		return string_t( "media/model/" ) + texture_name;
	}

	return texture_name;
}

/**
 * �`�悷��
 *
 */
void Direct3D11Mesh::render() const
{
	UINT buffer_count = vertex_buffer_list_.size();
	UINT stride[] = { sizeof( Vertex ), sizeof( SkinningInfo ) };
	UINT offset[] = { 0, 0 };

	if ( buffer_count < 2 )
	{
		return;
	}

	direct_3d_->getImmediateContext()->IASetVertexBuffers( 0, buffer_count, & vertex_buffer_list_[ 0 ], stride, offset );
	direct_3d_->getImmediateContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	for ( MaterialList::const_iterator i = material_list_.begin(); i != material_list_.end(); ++i )
	{
		( *i )->render();
	}
}