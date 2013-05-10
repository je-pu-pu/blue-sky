#include "FbxFileLoader.h"

#include "Direct3D11Mesh.h"
#include "Direct3D11Material.h"

// #include <game/GraphicsManager.h>

#include <map>

#include <iostream>
#include <cassert>

// #include <common/exception.h>
// #include <common/serialize.h>

#ifdef _DEBUG
#pragma comment ( lib, "fbxsdk-2013.3-mtd.lib" )
#else
#pragma comment ( lib, "fbxsdk-2013.3-mt.lib" )
#endif

void print_fbx_node_recursive( FbxNode* node )
{
	std::string s = node->GetName();

	std::cout << "Object : " << s << std::endl;

	FbxMesh* mesh = node->GetMesh();

	if ( mesh )
	{
		std::cout << "vertex : " << std::endl;
						
		for ( int n = 0; n < mesh->GetControlPointsCount(); n++ )
		{
				FbxVector4 v = mesh->GetControlPointAt( n );

				std::cout << n << " : ( " << v[ 0 ] << ", " << v[ 1 ] << ", " << v[ 2 ] << " )" << std::endl;
		}

		std::cout << "face : " << std::endl;

		for ( int n = 0; n < mesh->GetPolygonCount(); n++ )
		{
			std::cout << n << " : ( ";

			for ( int m = 0; m < mesh->GetPolygonSize( n ); m++ )
			{
				if ( m > 0 )
				{
					std::cout << ", ";
				}

				std::cout << mesh->GetPolygonVertex( n, m );
			}

			std::cout << " ) ( ";
			
			for ( int m = 0; m < mesh->GetPolygonSize( n ); m++ )
			{
				if ( m > 0 )
				{
					std::cout << ", ";
				}

				FbxVector2 uv_vector;
				
				if ( mesh->GetPolygonVertexUV( n, m, "UVMap", uv_vector ) )
				{
					std::cout << uv_vector[ 0 ] << "," << uv_vector[ 1 ];
				}

				FbxVector4 normal_vector;

				if ( mesh->GetPolygonVertexNormal( n, m, normal_vector ) )
				{
					std::cout <<
						normal_vector[ 0 ] << "," << 
						normal_vector[ 1 ] << "," << 
						normal_vector[ 2 ] << "," << 
						normal_vector[ 3 ] << "," << std::endl;
				}
			}
			
			std::cout << " )" << std::endl;
		}
		
		for ( int n = 0; n < mesh->GetLayerCount(); n++ )
		{
			FbxLayer* layer = mesh->GetLayer( n );

			FbxLayerElementNormal* normal = layer->GetNormals();
			
			if ( normal )
			{
				assert( normal->GetMappingMode() == FbxLayerElement::eByControlPoint );
				assert( normal->GetReferenceMode() == FbxLayerElement::eDirect );

				std::cout << "normal : " << normal->GetName() << " : " << std::endl;

				for ( int m = 0; m < normal->GetDirectArray().GetCount(); m++ )
				{
					std::cout << m << " : ( " <<
						normal->GetDirectArray().GetAt( m )[ 0 ] << ", " <<
						normal->GetDirectArray().GetAt( m )[ 1 ] << ", " <<
						normal->GetDirectArray().GetAt( m )[ 2 ] << " )" << std::endl;
				}
			}

			FbxLayerElementUV* uv = layer->GetUVs();

			if ( uv )
			{
				assert( uv->GetMappingMode() == FbxLayerElement::eByPolygonVertex );
				assert( uv->GetReferenceMode() == FbxLayerElement::eIndexToDirect );

				std::cout << "uv : " << uv->GetName() << " : " << std::endl;

				for ( int m = 0; m < uv->GetIndexArray().GetCount(); m++ )
				{
					int i = uv->GetIndexArray().GetAt( m );

					std::cout << m << " : ( " <<
						uv->GetDirectArray().GetAt( i )[ 0 ] << ", " <<
						uv->GetDirectArray().GetAt( i )[ 1 ] << " )" << std::endl;
				}
			}
		}

		for ( int n = 0; n < node->GetMaterialCount(); n++ )
		{
			FbxSurfaceMaterial* material = node->GetMaterial( n );

			if ( ! material )
			{
				continue;
			}

			std::cout << "material : " << material->GetName() << std::endl;

			for ( FbxProperty property = material->GetFirstProperty(); property.IsValid(); property = material->GetNextProperty( property ) )
			{
				std::cout << "property : " << property.GetName() << " ( texture count : " << property.GetSrcObjectCount< FbxTexture >() << " )" << std::endl;

				for ( int m = 0; m < property.GetSrcObjectCount< FbxTexture >(); m++ )
				{
					FbxLayeredTexture* layered_texture = property.GetSrcObject< FbxLayeredTexture >( m );

					if ( layered_texture )
					{
						for ( int l = 0; l < layered_texture->GetSrcObjectCount< FbxTexture >(); l++ )
						{
							if ( ! layered_texture->GetSrcObject< FbxTexture >( l ) )
							{
								continue;
							}

							std::cout << "texture " << l << " : " << layered_texture->GetSrcObject< FbxTexture >( l )->GetName() << std::endl;
						}
					}
					else
					{
						if ( ! property.GetSrcObject< FbxTexture >( m ) )
						{
							continue;
						}

						std::cout << "texture : " << property.GetSrcObject< FbxTexture >( m )->GetName() << std::endl;
						
						FbxFileTexture* file_texture = FbxCast< FbxFileTexture >( property.GetSrcObject< FbxTexture >( m ) );

						if ( file_texture )
						{
							std::cout << "file_name : " << file_texture->GetFileName() << std::endl;
							std::cout << "relative_file_name : " << file_texture->GetRelativeFileName() << std::endl;
						}
					}
				}
			}
		}
	}

	for ( int n = 0; n < node->GetChildCount(); n++ )
	{
		print_fbx_node_recursive( node->GetChild( n ) );
	}
}

FbxFileLoader::FbxFileLoader( Mesh* mesh )
	: mesh_( mesh )
{

}

bool FbxFileLoader::load( const char_t* file_name )
{
	FbxManager* sdk_manager = FbxManager::Create();
	FbxIOSettings* io_settings = FbxIOSettings::Create( sdk_manager, IOSROOT );
	sdk_manager->SetIOSettings( io_settings );

	FbxImporter* importer = FbxImporter::Create( sdk_manager, "" );
    
	if ( ! importer->Initialize( file_name, -1, sdk_manager->GetIOSettings() ) )
	{
		return false;
	}
    
	FbxScene* scene = FbxScene::Create( sdk_manager, "scene" );
	importer->Import( scene );
	importer->Destroy();
    
	FbxNode* root_node = scene->GetRootNode();

	if ( root_node )
	{
		for ( int n = 0; n < root_node->GetChildCount(); n++ )
		{
			// print_fbx_node_recursive( root_node->GetChild( n ) );
			load_node_recursive( root_node->GetChild( n ) );
		}
	}

	int n;
	FBXSDK_FOR_EACH_TEXTURE( n )
    {
		std::cout << n << " : " << FbxLayerElement::sTextureChannelNames[ n ] << std::endl;
	}

	sdk_manager->Destroy();

	// fix_coordinate_system()
	{
		for ( auto i = mesh_->get_vertex_list().begin(); i != mesh_->get_vertex_list().end(); ++i )
		{
			std::swap( i->Position.y, i->Position.z );
			std::swap( i->Normal.x, i->Normal.y );
		}

		for ( auto i = mesh_->get_material_list().begin(); i != mesh_->get_material_list().end(); ++i )
		{
			for ( uint_t n = 0; n < ( *i )->get_index_list().size(); ++n )
			{
				if ( n % 3 == 0 )
				{
					std::swap( ( *i )->get_index_list()[ n ], ( *i )->get_index_list()[ n + 1 ] );
				}
			}
		}
	}

	return true;
}

void FbxFileLoader::load_node_recursive( FbxNode* node )
{
	FbxMesh* mesh = node->GetMesh();

	if ( mesh )
	{
		typedef std::map< Mesh::Vertex, Material::Index > VertexIndexMap;
		typedef std::vector< Mesh::Vertex > VertexList;

		VertexIndexMap vertex_index_map;
		VertexList vertex_list;

		Mesh::PositionList position_list;
		Mesh::SkinningInfoList skinning_info_list;
		Mesh::Material* material = mesh_->create_material();

		mesh_->get_material_list().push_back( material );

		for ( int n = 0; n < mesh->GetControlPointsCount(); n++ )
		{
			FbxVector4 v = mesh->GetControlPointAt( n );

			position_list.push_back(
				Mesh::Position(
					static_cast< float >( v[ 0 ] ),
					static_cast< float >( v[ 1 ] ),
					static_cast< float >( v[ 2 ] ) ) );
		}

		// load_skinning_info()
		int skin_count = mesh->GetDeformerCount( FbxDeformer::eSkin );

		if ( skin_count > 0 )
		{
			assert( skin_count == 1 );

			skinning_info_list.resize( position_list.size() );

			for ( int n = 0; n < skin_count; ++n )
			{
				FbxSkin* skin = FbxCast< FbxSkin >( mesh->GetDeformer( n, FbxDeformer::eSkin ) );

				std::cout << skin->GetClusterCount() << " bones" << std::endl;

				for ( int bone_index = 0; bone_index < skin->GetClusterCount(); ++bone_index )
				{
					std::cout << "bone " << bone_index << " : " << std::endl;

					FbxCluster* cluster = skin->GetCluster( bone_index );

					for ( int i = 0; i < cluster->GetControlPointIndicesCount(); ++i )
					{
						int index = cluster->GetControlPointIndices()[ i ];
						float weight = static_cast< float >( cluster->GetControlPointWeights()[ i ] );

						skinning_info_list[ index ].add( bone_index, weight );

						std::cout << index << ":" << weight << std::endl;
					}
				}
			}
		}

		for ( int n = 0; n < mesh->GetPolygonCount(); n++ )
		{
			for ( int m = 0; m < mesh->GetPolygonSize( n ); m++ )
			{
				int position_index = mesh->GetPolygonVertex( n, m );

				Mesh::Vertex v;
				v.Position = Mesh::Position( position_list.at( position_index ) );

				FbxVector2 uv_vector;
				
				if ( mesh->GetPolygonVertexUV( n, m, "UVMap", uv_vector ) )
				{
					v.TexCoord = Mesh::TexCoord(
						static_cast< float >( uv_vector[ 0 ] ),
						1.f - static_cast< float >( uv_vector[ 1 ] ) );
				}

				FbxVector4 normal_vector;

				if ( mesh->GetPolygonVertexNormal( n, m, normal_vector ) )
				{
					v.Normal = Mesh::Normal(
						static_cast< float >( normal_vector[ 0 ] ),
						static_cast< float >( normal_vector[ 1 ] ),
						static_cast< float >( normal_vector[ 2 ] ) );
				}

				VertexIndexMap::iterator i = vertex_index_map.find( v );

				if ( i != vertex_index_map.end() )
				{
					material->get_index_list().push_back( i->second );
				}
				else
				{
					Material::Index vertex_index = mesh_->get_vertex_list().size();

					mesh_->get_vertex_list().push_back( v );

					if ( ! skinning_info_list.empty() )
					{
						mesh_->get_skinning_info_list().push_back( skinning_info_list.at( position_index ) );
					}

					material->get_index_list().push_back( vertex_index );

					vertex_index_map[ v ] = vertex_index;
				}
			}
		}

		string_t texture_file_path;

		for ( int n = 0; n < node->GetMaterialCount(); n++ )
		{
			FbxSurfaceMaterial* material = node->GetMaterial( n );

			if ( ! material )
			{
				continue;
			}

			for ( auto p = material->GetFirstProperty(); p.IsValid(); p = material->GetNextProperty( p ) )
			{
				for ( int m = 0; m < p.GetSrcObjectCount< FbxTexture >(); m++ )
				{
					FbxLayeredTexture* layered_texture = p.GetSrcObject< FbxLayeredTexture >( m );

					if ( layered_texture )
					{
						for ( int l = 0; l < layered_texture->GetSrcObjectCount< FbxTexture >(); l++ )
						{
							if ( ! layered_texture->GetSrcObject< FbxTexture >( l ) )
							{
								continue;
							}

							FbxFileTexture* file_texture = FbxCast< FbxFileTexture >( layered_texture->GetSrcObject< FbxTexture >( l ) );

							if ( file_texture )
							{
								texture_file_path = file_texture->GetFileName();

								break; /// !!!
							}
						}
					}
					else
					{
						if ( ! p.GetSrcObject< FbxTexture >( m ) )
						{
							continue;
						}

						FbxFileTexture* file_texture = FbxCast< FbxFileTexture >( p.GetSrcObject< FbxTexture >( m ) );

						if ( file_texture )
						{
							texture_file_path = file_texture->GetFileName();
						}
					}
				}
			}
		}

		if ( ! texture_file_path.empty() )
		{
			material->load_texture( texture_file_path.c_str() );
		}
	}

	for ( int n = 0; n < node->GetChildCount(); n++ )
	{
		load_node_recursive( node->GetChild( n ) );
	}
}
