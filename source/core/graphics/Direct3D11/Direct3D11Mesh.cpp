#include "Direct3D11Mesh.h"
#include "Direct3D11Material.h"
#include "Direct3D11TextureManager.h"
#include "Direct3D11.h"

#include "FbxFileLoader.h"
#include "SkinningAnimationSet.h"

#include <game/Shader.h>

#include <common/exception.h>
#include <common/math.h>
#include <common/log.h>

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
	
}

/**
 * �V�������_�O���[�v���쐬����
 *
 * @return �쐬���ꂽ���_�O���[�v
 */
Direct3D11Mesh::VertexGroup* Direct3D11Mesh::create_vertex_group()
{
	vertex_group_list_.push_back( std::make_unique< VertexGroup >() );

	return vertex_group_list_.back().get();
}

/**
 * ���_�O���[�v�̈ꗗ�̒�����w�肵���C���f�b�N�X�̒��_�O���[�v���擾����
 *
 * @param index ���_�O���[�v�̃C���f�b�N�X
 * @param force ���_�O���[�v�����݂��Ȃ��ꍇ�A�V�������_�O���[�v���쐬����t���O
 * @return ���_�O���[�v or 0
 */
Direct3D11Mesh::VertexGroup* Direct3D11Mesh::get_vertex_group_at( uint_t index, bool force )
{
	if ( index >= vertex_group_list_.size() )
	{
		if ( ! force )
		{
			return 0;
		}
		
		while ( index >= vertex_group_list_.size() )
		{
			create_vertex_group();
		}
	}

	return vertex_group_list_[ index ].get();
}

/**
 * �V�����}�e���A�����쐬����
 *
 * @return �쐬���ꂽ�}�e���A��
 */
Direct3D11Mesh::Material* Direct3D11Mesh::create_material()
{
	material_list_.push_back( std::make_unique< Direct3D11Material >() );
	// material_list_.back()->set_shader( new BypassShader() );

	return material_list_.back().get();
}

/**
 * �}�e���A���̈ꗗ�̒�����w�肵���C���f�b�N�X�̃}�e���A�����擾����
 *
 * @param index �}�e���A���̃C���f�b�N�X
 * @param force �}�e���A�������݂��Ȃ��ꍇ�A�V�����}�e���A�����쐬����t���O
 * @return �}�e���A�� or 0
 */
Direct3D11Mesh::Material* Direct3D11Mesh::get_material_at( uint_t index, bool force )
{
	if ( index >= material_list_.size() )
	{
		if ( ! force )
		{
			return 0;
		}
		
		while ( index >= material_list_.size() )
		{
			create_material();
		}
	}

	return material_list_[ index ].get();
}

/**
 * �}�e���A���̈ꗗ�̒�����w�肵���C���f�b�N�X�̃}�e���A�����擾����
 *
 * @param index �}�e���A���̃C���f�b�N�X
 * @return �}�e���A�� or 0
 */
Direct3D11Mesh::Material* Direct3D11Mesh::get_material_at( uint_t index )
{
	return get_material_at( index, false );
}

const Direct3D11Mesh::Material* Direct3D11Mesh::get_material_at( uint_t index ) const
{
	return material_list_[ index ].get();
}

/**
 * OBJ �t�@�C����ǂݍ���
 *
 * @todo Loader �Ƃ��ĕ�������
 *
 * @param file_name OBJ �t�@�C����
 * @return �t�@�C���̓ǂݍ��݂ɐ��������ꍇ�� true ���A���s�����ꍇ�� false ��Ԃ�
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
 * FBX �t�@�C����ǂݍ���
 *
 * @param file_name FBX �t�@�C����
 * @param loader FBX �t�@�C�����[�_�[
 * @return �t�@�C���̓ǂݍ��݂ɐ��������ꍇ�� true ���A���s�����ꍇ�� false ��Ԃ�
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
 * FBX �t�@�C����ǂݍ���
 *
 * @param file_name FBX �t�@�C����
 * @param loader FBX �t�@�C�����[�_�[
 * @param skinning_animation_set �����ɓǂݍ��ރX�L�j���O�A�j���[�V���������i�[����|�C���^
 * @return �t�@�C���̓ǂݍ��݂ɐ��������ꍇ�� true ���A���s�����ꍇ�� false ��Ԃ�
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

SkinningAnimationSet* Direct3D11Mesh::setup_skinning_animation_set()
{
	if ( ! skinning_animation_set_ )
	{
		skinning_animation_set_.reset( new SkinningAnimationSet() );
	}

	return skinning_animation_set_.get();
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
		ID3D11Buffer* buffer = 0;
		D3D11_BUFFER_DESC buffer_desc = { 0 };

		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		buffer_desc.ByteWidth = sizeof( Vertex ) * vertex_list_.size();
    
		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = & vertex_list_[ 0 ];
	
		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, & data, & buffer ) );

		vertex_buffer_list_.push_back( com_ptr< ID3D11Buffer >( buffer ) );
	}

	if ( ! skinning_info_list_.empty() )
	{
		ID3D11Buffer* buffer = 0;
		D3D11_BUFFER_DESC buffer_desc = { 0 };

		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		buffer_desc.ByteWidth = sizeof( SkinningInfo ) * skinning_info_list_.size();
    
		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = & skinning_info_list_[ 0 ];
	
		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, & data, & buffer ) );

		vertex_buffer_list_.push_back( com_ptr< ID3D11Buffer >( buffer ) );
	}
}

/**
 * �S�Ă̒��_�O���[�v�̃C���f�b�N�X�o�b�t�@���쐬����
 *
 */
void Direct3D11Mesh::create_index_buffer()
{
	index_buffer_list_.resize( vertex_group_list_.size() );

	for ( auto n = 0; n < vertex_group_list_.size(); n++ )
	{
		if ( vertex_group_list_[ n ]->empty() )
		{
			continue;
		}

		D3D11_BUFFER_DESC buffer_desc = { 0 };

		buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		buffer_desc.ByteWidth = sizeof( Index ) * vertex_group_list_[ n ]->size();

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = vertex_group_list_[ n ]->get_pointer();

		DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, & data, & index_buffer_list_[ n ] ) );

		index_count_list_.push_back( vertex_group_list_[ n ]->size() );
		
		get_material_at( n, true );
	}
}

/**
 * �V�X�e����������̒��_�����N���A����
 *
 */
void Direct3D11Mesh::clear_vertex_list()
{
	vertex_list_.clear();
}

/**
 * �V�X�e����������̃C���f�b�N�X�����N���A����
 *
 */
void Direct3D11Mesh::clear_vertex_group_list()
{
	vertex_group_list_.clear();
}

/**
 * �V�X�e����������̃X�L�j���O�����N���A����
 *
 */
void Direct3D11Mesh::clear_skinning_info_list()
{
	skinning_info_list_.clear();
}

/**
 * �w�肵���e�N�X�`�����ɑΉ�����e�N�X�`���t�@�C���p�X���擾����
 *
 * @param texture_name �e�N�X�`����
 * @return �e�N�X�`���t�@�C���p�X
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
 *  �w�肵���e�N�X�`�����ɑΉ�����e�N�X�`���t�@�C����ǂݍ���
 *
 * @todo �ǂ����ʂ̏ꏊ�Ɉړ����Đ�������
 *
 * @param texture_name �e�N�X�`����
 * @return �e�N�X�`��
 */
game::Texture* Direct3D11Mesh::load_texture_by_texture_name( const char_t* texture_name ) const
{
	auto texture_file_path = get_texture_file_path_by_texture_name( texture_name );
	return direct_3d_->getTextureManager()->load( texture_file_path.c_str(), texture_file_path.c_str() );
}

/**
 * �|���S���̗��\�𔽓]����
 *
 */
void Direct3D11Mesh::flip()
{
	for ( auto& vertex_group : vertex_group_list_ )
	{
		vertex_group->flip();
	}
}

/**
 * ���̓A�Z���u���ɐݒ肷��
 *
 */
void Direct3D11Mesh::bind() const
{
	if ( vertex_buffer_list_.empty() )
	{
		return;
	}

	UINT buffer_count = vertex_buffer_list_.size();
	UINT stride[] = { sizeof( Vertex ), sizeof( SkinningInfo ) };
	UINT offset[] = { 0, 0 };

	direct_3d_->getImmediateContext()->IASetVertexBuffers( 0, buffer_count, & vertex_buffer_list_[ 0 ], stride, offset );
}

/**
 * �`�悷��
 *
 */
void Direct3D11Mesh::render() const
{
	bind();

	int n = 0;

	for ( const auto& material : material_list_ )
	{
		material->get_shader()->render( this, n );

		n++;
	}
}

/**
 * ���b�V�����̃C���f�b�N�X�Ŏw�肵�����_�O���[�v�݂̂�`�悷��
 *
 * @param n �`�悷�钸�_�O���[�v�̃C���f�b�N�X
 */
void Direct3D11Mesh::render( uint_t n ) const
{
	direct_3d_->getImmediateContext()->IASetIndexBuffer( index_buffer_list_[ n ].get(), IndexBufferFormat, 0 );
	direct_3d_->getImmediateContext()->DrawIndexed( index_count_list_[ n ], 0, 0 );
}