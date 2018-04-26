#include "FbxFileLoader.h"

#include "SkinningAnimationSet.h"

#include <core/graphics/Direct3D11/Direct3D11Mesh.h>
#include <core/graphics/Direct3D11/Direct3D11Material.h>
#include <core/type.h>

#include <common/timer.h>
#include <common/exception.h>

#include <boost/filesystem/convenience.hpp>

#include <map>

#include <iostream>
#include <cassert>

#ifdef _DEBUG
#pragma comment ( lib, "libfbxsdk-mt.lib" )
#else
#pragma comment ( lib, "libfbxsdk-mt.lib" )
#endif

void print_fbx_node_recursive( FbxNode* node )
{
	std::string s = node->GetName();

	std::cout << "Object : " << s << std::endl;
	std::cout << "type : " << node->GetTypeName() << std::endl;

	FbxMesh* mesh = node->GetMesh();
	
	std::map< int_t, string_t > ro_map;
	ro_map[ FbxEuler::eOrderXYZ ] = "XYZ";
//	ro_map[ FbxEuler::eOrderXYX ] = "XYX";
	ro_map[ FbxEuler::eOrderXZY ] = "XZY";
//	ro_map[ FbxEuler::eOrderXZX ] = "XZX";
	ro_map[ FbxEuler::eOrderYZX ] = "YZX";
//	ro_map[ FbxEuler::eOrderYZY ] = "YZY";
	ro_map[ FbxEuler::eOrderYXZ ] = "YXZ";
//	ro_map[ FbxEuler::eOrderYXY ] = "YXY";
	ro_map[ FbxEuler::eOrderZXY ] = "ZXY";
//	ro_map[ FbxEuler::eOrderZXZ ] = "ZXZ";
	ro_map[ FbxEuler::eOrderZYX ] = "ZYX";
//	ro_map[ FbxEuler::eOrderZYZ ] = "ZYZ";

	std::cout << "rotation_order : " << ro_map[ node->RotationOrder.Get() ] << std::endl;

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
				bool unmapped;
				
				if ( mesh->GetPolygonVertexUV( n, m, "UVMap", uv_vector, unmapped ) )
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

			FbxLayerElementSmoothing* smooth = layer->GetSmoothing();

			if ( smooth )
			{
				assert( smooth->GetMappingMode() == FbxLayerElement::eByPolygon );
				assert( smooth->GetReferenceMode() == FbxLayerElement::eDirect );

				std::cout << "smooth : " << smooth->GetName() << " : " << std::endl;

				for ( int m= 0; m < smooth->GetDirectArray().GetCount(); m++ )
				{
					std::cout << m << " : " << smooth->GetDirectArray().GetAt( m ) << std::endl;
				}
			}

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

FbxFileLoader::FbxFileLoader()
	: fbx_manager_( FbxManager::Create() )
	, mesh_( 0 )
	, fbx_scene_( 0 )
	, fbx_material_index_( 0 )
{
	FbxIOSettings* io_settings = FbxIOSettings::Create( fbx_manager_, IOSROOT );
	fbx_manager_->SetIOSettings( io_settings );

	/*
	int n;
	FBXSDK_FOR_EACH_TEXTURE( n )
    {
		std::cout << n << " : " << FbxLayerElement::sTextureChannelNames[ n ] << std::endl;
	}
	*/
}

FbxFileLoader::~FbxFileLoader()
{
	fbx_manager_->Destroy();
}

/**
 * FBX ファイルを読み込む
 *
 * @param mesh 読込先のメッシュ
 * @param file_name FBX ファイル名
 * @return ファイルの読み込みに成功した場合は true を、失敗した場合は false を返す
 */
bool FbxFileLoader::load( Mesh* mesh, const char_t* file_name )
{
	mesh_ = mesh;
	fbx_material_index_ = 0;
	bone_index_map_.clear();

	common::timer t;

	if ( ! load( file_name ) )
	{
		return false;
	}

	std::cout << "imp " << t.elapsed() << std::endl;

	/*
	std::cout << "*** DirectX ***" << std::endl;
	print_axis_system( FbxAxisSystem::DirectX );

	std::cout << "*** FBX ***" << std::endl;
	print_axis_system( fbx_scene_->GetGlobalSettings().GetAxisSystem() );

	// FbxAxisSystem::DirectX.ConvertScene( fbx_scene_ );

	std::cout << "*** Converted FBX ***" << std::endl;
	print_axis_system( fbx_scene_->GetGlobalSettings().GetAxisSystem() );
	*/

	FbxNode* root_node = fbx_scene_->GetRootNode();

	if ( root_node )
	{
		/*
		for ( int n = 0; n < root_node->GetChildCount(); n++ )
		{
			print_fbx_node_recursive( root_node->GetChild( n ) );
		}
		*/

		t.restart();

		for ( int n = 0; n < root_node->GetChildCount(); n++ )
		{
			load_limb_recursive( root_node->GetChild( n ) );
		}

		std::cout << "limb " << t.elapsed() << std::endl;
		t.restart();

		load_animations();

		std::cout << "anim " << t.elapsed() << std::endl;
		t.restart();

		for ( int n = 0; n < root_node->GetChildCount(); n++ )
		{
			load_mesh_recursive( root_node->GetChild( n ) );
		}

		std::cout << "mesh " << t.elapsed() << std::endl;
		t.restart();
	}

	convert_coordinate_system();

	std::cout << "convert_coordinate_system " << t.elapsed() << std::endl;
	t.restart();

	return true;
}

/**
 * FbxNode からメッシュ情報を再帰的に読み込む
 *
 * @param node FbxNode
 */
void FbxFileLoader::load_mesh_recursive( FbxNode* node )
{
	load_mesh( node->GetMesh() );

	for ( int n = 0; n < node->GetMaterialCount(); n++ )
	{
		load_material( node->GetMaterial( n ) );
	}

	for ( int n = 0; n < node->GetChildCount(); n++ )
	{
		load_mesh_recursive( node->GetChild( n ) );
	}
}

/**
 * メッシュ情報を読み込む
 *
 * @param mesh メッシュ情報
 */
void FbxFileLoader::load_mesh( FbxMesh* mesh )
{
	if ( ! mesh )
	{
		return;
	}

	typedef std::map< Mesh::Vertex, Mesh::Material::Index > VertexIndexMap;
	typedef std::vector< Mesh::Vertex > VertexList;

	VertexIndexMap vertex_index_map;
	VertexList vertex_list;

	Mesh::PositionList position_list;
	Mesh::SkinningInfoList skinning_info_list;

	// load_mesh_vertex()
	for ( int n = 0; n < mesh->GetControlPointsCount(); n++ )
	{
		FbxVector4 v = mesh->GetControlPointAt( n );

		position_list.push_back(
			Mesh::Position(
				static_cast< float >( v[ 0 ] ),
				static_cast< float >( v[ 1 ] ),
				static_cast< float >( v[ 2 ] ) ) );
	}

	// load_mesh_skinning_info()
	{
		int skin_count = mesh->GetDeformerCount( FbxDeformer::eSkin );

		if ( skin_count > 0 )
		{
			assert( skin_count == 1 );

			skinning_info_list.resize( position_list.size() );

			FbxSkin* skin = FbxCast< FbxSkin >( mesh->GetDeformer( 0, FbxDeformer::eSkin ) );

			load_mesh_skinning_info( skin, skinning_info_list );
		}
	}

	FbxLayerElementSmoothing* smoothing = 0;

	// load_mesh_smoothing_info()
	{
		FbxLayer* layer = mesh->GetLayer( 0 );
		smoothing = layer->GetSmoothing();
	}

	if ( ! smoothing )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( "this FBX format is not supported. ( no smoothing info )" );
	}

	// load_mesh_plygon()
	FbxLayerElementArrayTemplate< int >* material_indices;
	mesh->GetMaterialIndices( & material_indices );

	for ( int n = 0; n < mesh->GetPolygonCount(); n++ )
	{
		Mesh::Material* material = mesh_->get_material_at( material_indices->GetAt( n ) );
		std::vector< Mesh::Vertex > v_tmp_list;

		bool is_smooth = smoothing->GetDirectArray().GetAt( n ) != 0;
		FbxVector4 polygon_normal( 0.f, 0.f, 0.f );

		if ( ! is_smooth )
		{
			// ポリゴンの法線を計算する
			Mesh::Position p1 = Mesh::Position( position_list.at( mesh->GetPolygonVertex( n, 0 ) ) );
			Mesh::Position p2 = Mesh::Position( position_list.at( mesh->GetPolygonVertex( n, 1 ) ) );
			Mesh::Position p3 = Mesh::Position( position_list.at( mesh->GetPolygonVertex( n, 2 ) ) );

			FbxVector4 a = FbxVector4( p1.x, p1.y, p1.z );
			FbxVector4 b = FbxVector4( p2.x, p2.y, p2.z );
			FbxVector4 c = FbxVector4( p3.x, p3.y, p3.z );

			FbxVector4 ab( b - a );
			FbxVector4 bc( c - b );

			polygon_normal = ab.CrossProduct( bc );
			polygon_normal.Normalize();
		}

		for ( int m = 0; m < mesh->GetPolygonSize( n ); m++ )
		{
			int position_index = mesh->GetPolygonVertex( n, m );

			Mesh::Vertex v;
			v.Position = Mesh::Position( position_list.at( position_index ) );

			FbxVector2 uv_vector;
			bool unmapped;
			
			if ( mesh->GetPolygonVertexUV( n, m, "UVMap", uv_vector, unmapped) )
			{
				v.TexCoord = Mesh::TexCoord(
					static_cast< float >( uv_vector[ 0 ] ),
					1.f - static_cast< float >( uv_vector[ 1 ] ) );
			}

			if ( is_smooth )
			{
				FbxVector4 normal_vector;

				// 頂点の法線
				if ( mesh->GetPolygonVertexNormal( n, m, normal_vector ) )
				{
					v.Normal = Mesh::Normal(
						static_cast< float >( normal_vector[ 0 ] ),
						static_cast< float >( normal_vector[ 1 ] ),
						static_cast< float >( normal_vector[ 2 ] ) );
				}
			}
			else
			{
				// ポリゴンの法線
				v.Normal = Mesh::Normal(
					static_cast< float >( polygon_normal[ 0 ] ),
					static_cast< float >( polygon_normal[ 1 ] ),
					static_cast< float >( polygon_normal[ 2 ] ) );
			}
			
			// 頂点の一覧に追加
			{
				VertexIndexMap::iterator i = vertex_index_map.find( v );

				if ( i != vertex_index_map.end() )
				{
					material->get_index_list().push_back( i->second );
				}
				else
				{
					Mesh::Material::Index vertex_index = static_cast< Mesh::Material::Index >( mesh_->get_vertex_list().size() );

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
	}
}

/**
 * メッシュのスキニング情報を読み込む
 *
 * @param mesh メッシュ情報
 * @param skinning_info_list スキニング情報への参照
 */
void FbxFileLoader::load_mesh_skinning_info( FbxSkin* skin, Mesh::SkinningInfoList& skinning_info_list )
{
	std::cout << skin->GetClusterCount() << " bones" << std::endl;

	for ( int cluster_index = 0; cluster_index < skin->GetClusterCount(); ++cluster_index )
	{
		FbxCluster* cluster = skin->GetCluster( cluster_index );
		std::cout << "cluster : " << cluster_index << " : " << cluster->GetNameOnly() << std::endl;
		std::cout << "link : " << cluster->GetLink()->GetName() << std::endl;
		std::cout << "type : " << cluster->GetLink()->GetTypeName() << std::endl;
		std::cout << "link mode : " << cluster->GetLinkMode() << std::endl;

		assert( cluster->GetLinkMode() == FbxCluster::eNormalize );

		int bone_index = 0;

		{
			auto i = bone_index_map_.find( cluster->GetLink() );

			if ( i == bone_index_map_.end() )
			{
				continue;
			}

			bone_index = i->second;
		}

		for ( int i = 0; i < cluster->GetControlPointIndicesCount(); ++i )
		{
			int index = cluster->GetControlPointIndices()[ i ];
			float weight = static_cast< float >( cluster->GetControlPointWeights()[ i ] );

			skinning_info_list[ index ].add( bone_index, weight );

			// std::cout << index << ":" << weight << std::endl;
		}

		FbxAMatrix initial_matrix;
		cluster->GetTransformLinkMatrix( initial_matrix );

		Matrix r, s, t;

		r.set_rotation_xyz(
			math::degree_to_radian( static_cast< float_t >( -initial_matrix.GetR()[ 0 ] ) ),
			math::degree_to_radian( static_cast< float_t >( -initial_matrix.GetR()[ 1 ] ) ),
			math::degree_to_radian( static_cast< float_t >(  initial_matrix.GetR()[ 2 ] ) ) );

		s.set_scaling(
			static_cast< float_t >( initial_matrix.GetS()[ 0 ] ),
			static_cast< float_t >( initial_matrix.GetS()[ 1 ] ),
			static_cast< float_t >( initial_matrix.GetS()[ 2 ] ) );

		t.set_translation(
			static_cast< float_t >(  initial_matrix.GetT()[ 0 ] ),
			static_cast< float_t >(  initial_matrix.GetT()[ 1 ] ),
			static_cast< float_t >( -initial_matrix.GetT()[ 2 ] ) );

		mesh_->get_skinning_animation_set()->get_bone_offset_matrix_by_bone_index( bone_index ) = s * r * t;
	}

	if ( mesh_->get_skinning_animation_set() )
	{
		mesh_->get_skinning_animation_set()->optimize();
		mesh_->get_skinning_animation_set()->calculate_length();
	}
}

/**
 * マテリアル情報を読み込む
 *
 * @param fbx_material マテリアル情報
 */
void FbxFileLoader::load_material( FbxSurfaceMaterial* fbx_material )
{
	if ( ! fbx_material )
	{
		return;
	}

	Mesh::Material* material = mesh_->get_material_at( fbx_material_index_ );

	fbx_material_index_++;

	string_t texture_file_path;

	for ( auto p = fbx_material->GetFirstProperty(); p.IsValid(); p = fbx_material->GetNextProperty( p ) )
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
						texture_file_path = convert_file_path_to_internal_encoding( file_texture->GetRelativeFileName() );

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
					texture_file_path = convert_file_path_to_internal_encoding( file_texture->GetRelativeFileName() );
				}
			}
		}
	}

	if ( ! texture_file_path.empty() )
	{
		texture_file_path = "media/model/" + texture_file_path;

		material->load_texture( texture_file_path.c_str() );
	}
}

/**
 * FbxNode からボーン情報を再帰的に読み込む
 *
 * @param node FbxNode
 */
void FbxFileLoader::load_limb_recursive( FbxNode* node )
{
	load_limb( node );

	for ( int n = 0; n < node->GetChildCount(); n++ )
	{
		load_limb_recursive( node->GetChild( n ) );
	}
}

/**
 * スケルトン情報を読み込む
 *
 */
void FbxFileLoader::load_limb( FbxNode* node )
{
	if ( std::string( node->GetTypeName() ) != "Limb" )
	{
		return;
	}

	if ( std::string( node->GetParent()->GetTypeName() ) != "Limb" )
	{
		return;
	}

	std::cout << "limb : " << node->GetName() << std::endl;

	bone_index_map_[ node ] = bone_index_map_.size();
}

/**
 * アニメーション情報を読み込む
 *
 */
void FbxFileLoader::load_animations()
{
	if ( bone_index_map_.empty() )
	{
		return;
	}

	SkinningAnimationSet* skinning_animation_set = mesh_->setup_skinning_animation_set();
	skinning_animation_set->set_bone_count( bone_index_map_.size() );

	for ( auto i = bone_index_map_.begin(); i != bone_index_map_.end(); ++i )
	{
		for ( int n = 0; n < i->first->GetChildCount(); ++n )
		{
			auto j = bone_index_map_.find( i->first->GetChild( n ) );

			if ( j != bone_index_map_.end() )
			{
				skinning_animation_set->add_child_bone_index( i->second, j->second );
			}
		}
	}

	for ( auto i = bone_index_map_.begin(); i != bone_index_map_.end(); ++i )
	{
		load_animations_for_bone( i->second, i->first );
	}
}

/**
 * 指定したインデックスのボーンに対応するアニメーションの一覧を読み込む
 *
 * @param bone_index ボーンのインデックス
 * @param cluster FbxCluster
 */
void FbxFileLoader::load_animations_for_bone( int bone_index, FbxNode* node )
{
	if ( ! node )
	{
		return;
	}

	for ( int n = 0; n < fbx_scene_->GetSrcObjectCount< FbxAnimStack >(); n++ )
	{
		FbxAnimStack* anim_stack = fbx_scene_->GetSrcObject< FbxAnimStack >( n );

		if ( ! anim_stack )
		{
			continue;
		}

		Animation& animation = mesh_->get_skinning_animation_set()->get_skinning_animation( anim_stack->GetName() ).get_bone_animation_by_bone_index( bone_index );
		animation.set_name( node->GetName() );

		for ( int m = 0; m < anim_stack->GetMemberCount< FbxAnimLayer >(); m++ )
		{
			FbxAnimLayer* anim_layer = anim_stack->GetMember< FbxAnimLayer >( m );

			if ( ! anim_layer )
			{
				continue;
			}

			load_curve_for_animation( animation, Animation::CHANNEL_INDEX_TX, node->LclTranslation.GetCurve( anim_layer, FBXSDK_CURVENODE_COMPONENT_X ) );
			load_curve_for_animation( animation, Animation::CHANNEL_INDEX_TY, node->LclTranslation.GetCurve( anim_layer, FBXSDK_CURVENODE_COMPONENT_Y ) );
			load_curve_for_animation( animation, Animation::CHANNEL_INDEX_TZ, node->LclTranslation.GetCurve( anim_layer, FBXSDK_CURVENODE_COMPONENT_Z ) );

			load_curve_for_animation( animation, Animation::CHANNEL_INDEX_RX, node->LclRotation.GetCurve( anim_layer, FBXSDK_CURVENODE_COMPONENT_X ) );
			load_curve_for_animation( animation, Animation::CHANNEL_INDEX_RY, node->LclRotation.GetCurve( anim_layer, FBXSDK_CURVENODE_COMPONENT_Y ) );
			load_curve_for_animation( animation, Animation::CHANNEL_INDEX_RZ, node->LclRotation.GetCurve( anim_layer, FBXSDK_CURVENODE_COMPONENT_Z ) );

			load_curve_for_animation( animation, Animation::CHANNEL_INDEX_SX, node->LclScaling.GetCurve( anim_layer, FBXSDK_CURVENODE_COMPONENT_X ) );
			load_curve_for_animation( animation, Animation::CHANNEL_INDEX_SY, node->LclScaling.GetCurve( anim_layer, FBXSDK_CURVENODE_COMPONENT_Y ) );
			load_curve_for_animation( animation, Animation::CHANNEL_INDEX_SZ, node->LclScaling.GetCurve( anim_layer, FBXSDK_CURVENODE_COMPONENT_Z ) );

			/*
			std::for_each(
				animation.get_channel( "rx" ).get_key_frame_list().begin(),
				animation.get_channel( "rx" ).get_key_frame_list().end(),
				[] ( AnimationKeyFrame& key_frame ) { std::cout << "\trx : " << key_frame.get_frame() << " : " << key_frame.get_value() << std::endl; } );

			std::for_each(
				animation.get_channel( "ry" ).get_key_frame_list().begin(),
				animation.get_channel( "ry" ).get_key_frame_list().end(),
				[] ( AnimationKeyFrame& key_frame ) { std::cout << "\try : " << key_frame.get_frame() << " : " << key_frame.get_value() << std::endl; } );

			std::for_each(
				animation.get_channel( "rz" ).get_key_frame_list().begin(),
				animation.get_channel( "rz" ).get_key_frame_list().end(),
				[] ( AnimationKeyFrame& key_frame ) { std::cout << "\trz : " << key_frame.get_frame() << " : " << key_frame.get_value() << std::endl; } );
			*/

			const Animation::ChannelIndex rotate_channel_indexes[] = {
				Animation::CHANNEL_INDEX_RX,
				Animation::CHANNEL_INDEX_RY,
				Animation::CHANNEL_INDEX_RZ
			};

			for ( int o = 0; o < 3; ++o )
			{
				std::for_each(
					animation.get_channel( rotate_channel_indexes[ o ] ).get_key_frame_list().begin(),
					animation.get_channel( rotate_channel_indexes[ o ] ).get_key_frame_list().end(),
					[] ( AnimationKeyFrame& key_frame ) { key_frame.get_value() = math::degree_to_radian( key_frame.get_value() ); } );
			}

			const Animation::ChannelIndex inverse_channel_indexes[] = {
				Animation::CHANNEL_INDEX_TZ,
				Animation::CHANNEL_INDEX_RX,
				Animation::CHANNEL_INDEX_RY
			};

			for ( int o = 0; o < 3; ++o )
			{
				std::for_each(
					animation.get_channel( inverse_channel_indexes[ o ] ).get_key_frame_list().begin(),
					animation.get_channel( inverse_channel_indexes[ o ] ).get_key_frame_list().end(),
					[] ( AnimationKeyFrame& key_frame ) { key_frame.get_value() = -key_frame.get_value(); } );
			}
		}
	}
}

/**
 * 指定したアニメーションにアニメーションカーブを読み込む
 *
 * @param animation アニメーション
 * @param channel_index アニメーションのチャンネルのインデックス
 * @param anim_curve FbxAnimCurve
 */
void FbxFileLoader::load_curve_for_animation( Animation& animation, Animation::ChannelIndex channel_index, const FbxAnimCurve* anim_curve )
{
	if ( ! anim_curve )
	{
		return;
	}

	for ( int l = 0; l < anim_curve->KeyGetCount(); l++ )
	{
		AnimationKeyFrame key_frame( 
			static_cast< float >( anim_curve->KeyGetTime( l ).GetFrameCount() ),
			static_cast< float >( anim_curve->KeyGetValue( l ) ) );

		animation.get_channel( channel_index ).add_key_frame( key_frame );
	}
}

/**
 * 座標系を変換する
 *
 */
void FbxFileLoader::convert_coordinate_system()
{
	for ( auto i = mesh_->get_vertex_list().begin(); i != mesh_->get_vertex_list().end(); ++i )
	{
		i->Position.z = -i->Position.z;
		i->Normal.z = -i->Normal.z;

		// std::swap( i->Position.y, i->Position.z );
		// std::swap( i->Normal.y, i->Normal.z );
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

/**
 * 相対パスのファイル名をテクスチャのファイル名として再帰的に設定する
 *
 * @param node FbxNode
 */
void FbxFileLoader::set_relative_file_name_to_texture_file_name_recursive( FbxNode* node )
{
	if ( ! node )
	{
		return;
	}

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
				FbxFileTexture* file_texture = nullptr;

				if ( layered_texture )
				{
					for ( int l = 0; l < layered_texture->GetSrcObjectCount< FbxTexture >(); l++ )
					{
						if ( ! layered_texture->GetSrcObject< FbxTexture >( l ) )
						{
							continue;
						}

						file_texture = FbxCast< FbxFileTexture >( layered_texture->GetSrcObject< FbxTexture >( l ) );
					}
				}
				else
				{
					if ( ! p.GetSrcObject< FbxTexture >( m ) )
					{
						continue;
					}

					file_texture = FbxCast< FbxFileTexture >( p.GetSrcObject< FbxTexture >( m ) );
				}

				if ( file_texture )
				{
					const string_t relative_file_name = file_texture->GetRelativeFileName();

					if ( ! file_texture->SetFileName( relative_file_name.c_str() ) )
					{
						COMMON_THROW_EXCEPTION_MESSAGE( string_t( "FbxFileTexture::SetFileName failed : " ) + relative_file_name );
					}
					/*
					if ( ! file_texture->SetRelativeFileName( relative_file_name.c_str() ) )
					{
						COMMON_THROW_EXCEPTION_MESSAGE( string_t( "FbxFileTexture::SetRelativeFileName failed : " ) + relative_file_name );
					}
					*/
				}
			}
		}
	}

	for ( int n = 0; n < node->GetChildCount(); n++ )
	{
		set_relative_file_name_to_texture_file_name_recursive( node->GetChild( n ) );
	}
}

void FbxFileLoader::print_local_transform( const FbxNode* node ) const
{
	std::cout.setf( std::ios_base::fixed, std::ios_base::floatfield );
	std::cout.precision( 4 );

	std::cout << "\tT : " << node->LclTranslation.Get()[ 0 ] << ", " << node->LclTranslation.Get()[ 1 ] << ", " << node->LclTranslation.Get()[ 2 ] << std::endl;
	std::cout << "\tR : " << node->LclRotation.Get()[ 0 ] << ", " << node->LclRotation.Get()[ 1 ] << ", " << node->LclRotation.Get()[ 2 ] << std::endl;
	std::cout << "\tS : " << node->LclScaling.Get()[ 0 ] << ", " << node->LclScaling.Get()[ 1 ] << ", " << node->LclScaling.Get()[ 2 ] << std::endl;
}

/**
 *
 *
 */
void FbxFileLoader::print_matrix( const FbxAMatrix& m ) const
{
	std::cout.setf( std::ios_base::fixed, std::ios_base::floatfield );
	std::cout.precision( 4 );

	std::cout << "\tT : " << m.GetT()[ 0 ] << ", " << m.GetT()[ 1 ] << ", " << m.GetT()[ 2 ] << std::endl;
	std::cout << "\tR : " << m.GetR()[ 0 ] << ", " << m.GetR()[ 1 ] << ", " << m.GetR()[ 2 ] << std::endl;
	std::cout << "\tS : " << m.GetS()[ 0 ] << ", " << m.GetS()[ 1 ] << ", " << m.GetS()[ 2 ] << std::endl;
}

/**
 * 座標系を表示する
 *
 */
void FbxFileLoader::print_axis_system( const FbxAxisSystem& axis_system ) const
{
	int front_sign, up_sign;

	FbxAxisSystem::EFrontVector front = axis_system.GetFrontVector( front_sign );
	FbxAxisSystem::EUpVector up = axis_system.GetUpVector( up_sign );
	FbxAxisSystem::ECoordSystem coord_system = axis_system.GetCoorSystem();

	std::cout << "up : ";
		
	if ( up_sign < 0 )
	{
		std::cout << "-";
	}

	if ( up == FbxAxisSystem::eXAxis )
	{
		std::cout << "X";
	}
	else if ( up == FbxAxisSystem::eYAxis )
	{
		std::cout << "Y";
	}
	else if ( up == FbxAxisSystem::eZAxis )
	{
		std::cout << "Z";
	}

	std::cout << std::endl;

	std::cout << "front : ";

	if ( front_sign < 0 )
	{
		std::cout << "-";
	}

	if ( up == FbxAxisSystem::eXAxis )
	{
		std::cout << ( front == FbxAxisSystem::eParityEven ? "Y" : "Z" );
	}
	else if ( up == FbxAxisSystem::eYAxis )
	{
		std::cout << ( front == FbxAxisSystem::eParityEven ? "X" : "Z" );
	}
	else if ( up == FbxAxisSystem::eZAxis )
	{
		std::cout << ( front == FbxAxisSystem::eParityEven ? "X" : "Y" );
	}

	std::cout << std::endl;

	std::cout << "coord system : ";
	
	if ( coord_system == FbxAxisSystem::eLeftHanded )
	{
		std::cout << "LeftHanded";
	}
	else if ( coord_system == FbxAxisSystem::eRightHanded )
	{
		std::cout << "RightHanded";
	}
	
	std::cout << std::endl;
}

/**
 * FBX ファイルをシーンに読み込む
 *
 * @param file_path FBX ファイルパス
 */
bool FbxFileLoader::load( const char_t* file_path )
{
	bool result = false;

	if ( fbx_scene_ )
	{
		fbx_scene_->Destroy( true );
	}

	fbx_scene_ = FbxScene::Create( fbx_manager_, "scene" );

	FbxImporter* importer = FbxImporter::Create( fbx_manager_, "" );
    
	if ( importer->Initialize( file_path, -1, fbx_manager_->GetIOSettings() ) )
	{
		result = importer->Import( fbx_scene_ );
	}
	
	importer->Destroy();

	FbxGeometryConverter converter( fbx_manager_ );
	converter.Triangulate( fbx_scene_, true );

	return result;
}

/**
 * 現在のシーンを FBX ファイルに保存する
 *
 * @param file_path FBX ファイルパス
 */
bool FbxFileLoader::save( const char_t* file_path )
{
	bool result = false;

	FbxExporter* exporter = FbxExporter::Create( fbx_manager_, "" );
	
	// if ( exporter->Initialize( boost::filesystem::absolute( file_path ).string().c_str(), -1, fbx_manager_->GetIOSettings() ) )
	if ( exporter->Initialize( file_path, -1, fbx_manager_->GetIOSettings() ) )
	{
		result = exporter->Export( fbx_scene_ );
	}

	exporter->Destroy();

	return result;
}

/**
 * 指定した FBX ファイルを読み込みバイナリ形式で保存する
 *
 * 
 * @param file_path 読み込む FBX ファイルのパス
 * @param binary_file_path バイナリ形で保存する FBX ファイルのパス
 * @return FBX ファイルの保存に成功した場合は true を、失敗した場合は false を返す
 */
bool FbxFileLoader::convert_to_binaly( const char_t* file_path, const char_t* binary_file_path )
{
	if ( ! load( file_path ) )
	{
		return false;
	}

	set_relative_file_name_to_texture_file_name_recursive( fbx_scene_->GetRootNode() );

	return save( binary_file_path );
}

/**
 * FBX から取得したファイルパス文字列を内部エンコーディングに変換する
 *
 * @param s FBX から取得した文字列
 */
string_t FbxFileLoader::convert_file_path_to_internal_encoding( const char* s )
{
	char* path = 0;

	// FbxAnsiToUTF8( file_texture->GetFileName(), path, & buffer_size );
	FbxUTF8ToAnsi( s, path );

	string_t result( path );

	delete [] path;

	return result;
}