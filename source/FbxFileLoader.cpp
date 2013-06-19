#include "FbxFileLoader.h"

#include "Direct3D11Mesh.h"
#include "Direct3D11Material.h"
#include "Direct3D11Matrix.h"
#include "SkinningAnimationSet.h"

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
	std::cout << "type : " << node->GetTypeName() << std::endl;

	FbxMesh* mesh = node->GetMesh();
	
	std::map< int_t, string_t > ro_map;
	ro_map[ FbxEuler::eOrderXYZ ] = "XYZ";
	ro_map[ FbxEuler::eOrderXYX ] = "XYX";
	ro_map[ FbxEuler::eOrderXZY ] = "XZY";
	ro_map[ FbxEuler::eOrderXZX ] = "XZX";
	ro_map[ FbxEuler::eOrderYZX ] = "YZX";
	ro_map[ FbxEuler::eOrderYZY ] = "YZY";
	ro_map[ FbxEuler::eOrderYXZ ] = "YXZ";
	ro_map[ FbxEuler::eOrderYXY ] = "YXY";
	ro_map[ FbxEuler::eOrderZXY ] = "ZXY";
	ro_map[ FbxEuler::eOrderZXZ ] = "ZXZ";
	ro_map[ FbxEuler::eOrderZYX ] = "ZYX";
	ro_map[ FbxEuler::eOrderZYZ ] = "ZYZ";

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
	: fbx_manager_( FbxManager::Create() )
	, mesh_( mesh )
	, fbx_scene_( 0 )
	, fbx_material_index_( 0 )
{
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
 * @param file_name FBX ファイル名
 * @return ファイルの読み込みに成功した場合は true を、失敗した場合は false を返す
 */
bool FbxFileLoader::load( const char_t* file_name )
{
	FbxIOSettings* io_settings = FbxIOSettings::Create( fbx_manager_, IOSROOT );
	fbx_manager_->SetIOSettings( io_settings );

	FbxImporter* importer = FbxImporter::Create( fbx_manager_, "" );
    
	if ( ! importer->Initialize( file_name, -1, fbx_manager_->GetIOSettings() ) )
	{
		return false;
	}
    
	fbx_scene_ = FbxScene::Create( fbx_manager_, "scene" );
	importer->Import( fbx_scene_ );
	importer->Destroy();
    
	std::cout << "*** DirectX ***" << std::endl;
	print_axis_system( FbxAxisSystem::DirectX );

	std::cout << "*** FBX ***" << std::endl;
	print_axis_system( fbx_scene_->GetGlobalSettings().GetAxisSystem() );

	// FbxAxisSystem::DirectX.ConvertScene( fbx_scene_ );

	std::cout << "*** Converted FBX ***" << std::endl;
	print_axis_system( fbx_scene_->GetGlobalSettings().GetAxisSystem() );

	FbxNode* root_node = fbx_scene_->GetRootNode();

	if ( root_node )
	{
		/*
		for ( int n = 0; n < root_node->GetChildCount(); n++ )
		{
			print_fbx_node_recursive( root_node->GetChild( n ) );
		}
		*/

		for ( int n = 0; n < root_node->GetChildCount(); n++ )
		{
			load_limb_recursive( root_node->GetChild( n ) );
		}

		for ( int n = 0; n < root_node->GetChildCount(); n++ )
		{
			load_mesh_recursive( root_node->GetChild( n ) );
		}
	}

//	convert_coordinate_system();

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

	typedef std::map< Mesh::Vertex, Material::Index > VertexIndexMap;
	typedef std::vector< Mesh::Vertex > VertexList;

	VertexIndexMap vertex_index_map;
	VertexList vertex_list;

	Mesh::PositionList position_list;
	Mesh::SkinningInfoList skinning_info_list;

	for ( int n = 0; n < mesh->GetControlPointsCount(); n++ )
	{
		FbxVector4 v = mesh->GetControlPointAt( n );

		position_list.push_back(
			Mesh::Position(
				static_cast< float >( v[ 0 ] ),
				static_cast< float >( v[ 1 ] ),
				static_cast< float >( v[ 2 ] ) ) );
	}

	FbxLayerElementArrayTemplate< int >* material_indices;
	mesh->GetMaterialIndices( & material_indices );

	// load_skinning_info()
	int skin_count = mesh->GetDeformerCount( FbxDeformer::eSkin );

	if ( skin_count > 0 )
	{
		assert( skin_count == 1 );

		skinning_info_list.resize( position_list.size() );
			
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

		for ( int n = 0; n < skin_count; ++n )
		{
			FbxSkin* skin = FbxCast< FbxSkin >( mesh->GetDeformer( n, FbxDeformer::eSkin ) );

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

				load_animations_for_bone( bone_index, cluster );

				for ( int i = 0; i < cluster->GetControlPointIndicesCount(); ++i )
				{
					int index = cluster->GetControlPointIndices()[ i ];
					float weight = static_cast< float >( cluster->GetControlPointWeights()[ i ] );

					skinning_info_list[ index ].add( bone_index, weight );

					// std::cout << index << ":" << weight << std::endl;
				}

				FbxAMatrix transform_matrix, transform_link_matrix;

				cluster->GetTransformMatrix( transform_matrix );
				cluster->GetTransformLinkMatrix( transform_link_matrix );

				const FbxAMatrix& node_global_transform = fbx_scene_->GetEvaluator()->GetNodeLocalTransform( cluster->GetLink() );

				std::cout << "LocalTransform : " << std::endl;
				print_local_transform( cluster->GetLink() );

				// std::cout << "NodeGlobalTransform : " << std::endl;
				// print_matrix( node_global_transform );

				std::cout << "TransformMatrix : " << std::endl;
				print_matrix( transform_matrix );

				std::cout << "TransformLinkMatrix : " << std::endl;
				print_matrix( transform_link_matrix );

				FbxAMatrix initial_matrix = transform_link_matrix; // .Inverse() * transform_matrix;

				std::cout << "BoneInitialMatrix : " << std::endl;
				print_matrix( initial_matrix );

				// initial_matrix.SetIdentity();

				skinning_animation_set->get_bone_offset_matrix_by_bone_index( bone_index ).set(
					static_cast< float_t >( initial_matrix.Get( 0, 0 ) ), static_cast< float_t >( initial_matrix.Get( 0, 1 ) ), static_cast< float_t >( initial_matrix.Get( 0, 2 ) ), static_cast< float_t >( initial_matrix.Get( 0, 3 ) ),
					static_cast< float_t >( initial_matrix.Get( 1, 0 ) ), static_cast< float_t >( initial_matrix.Get( 1, 1 ) ), static_cast< float_t >( initial_matrix.Get( 1, 2 ) ), static_cast< float_t >( initial_matrix.Get( 1, 3 ) ),
					static_cast< float_t >( initial_matrix.Get( 2, 0 ) ), static_cast< float_t >( initial_matrix.Get( 2, 1 ) ), static_cast< float_t >( initial_matrix.Get( 2, 2 ) ), static_cast< float_t >( initial_matrix.Get( 2, 3 ) ),
					static_cast< float_t >( initial_matrix.Get( 3, 0 ) ), static_cast< float_t >( initial_matrix.Get( 3, 1 ) ), static_cast< float_t >( initial_matrix.Get( 3, 2 ) ), static_cast< float_t >( initial_matrix.Get( 3, 3 ) ) );

				/*
				Matrix r, s, t;

				r.set_rotation(
					math::degree_to_radian( static_cast< float_t >( initial_matrix.GetR()[ 0 ] ) ),
					math::degree_to_radian( static_cast< float_t >( initial_matrix.GetR()[ 1 ] ) ),
					math::degree_to_radian( static_cast< float_t >( initial_matrix.GetR()[ 2 ] ) ) );

				s.set_scaling(
					static_cast< float_t >( initial_matrix.GetS()[ 0 ] ),
					static_cast< float_t >( initial_matrix.GetS()[ 1 ] ),
					static_cast< float_t >( initial_matrix.GetS()[ 2 ] ) );

				t.set_translation(
					static_cast< float_t >( initial_matrix.GetT()[ 0 ] ),
					static_cast< float_t >( initial_matrix.GetT()[ 1 ] ),
					static_cast< float_t >( initial_matrix.GetT()[ 2 ] ) );

				skinning_animation_set->get_bone_offset_matrix_by_bone_index( bone_index ) = s * r * t;
				*/
			}
		}

		// skinning_animation_set->get_bone_offset_matrix_by_bone_index( 0 ).set_identity();
		// skinning_animation_set->get_bone_offset_matrix_by_bone_index( 0 ).set_identity();

		skinning_animation_set->optimize();
		skinning_animation_set->calculate_length();
	}

	for ( int n = 0; n < mesh->GetPolygonCount(); n++ )
	{
		Mesh::Material* material = mesh_->get_material_at( material_indices->GetAt( n ) );

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

	if ( ! texture_file_path.empty() )
	{
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
 * 指定したインデックスのボーンに対応するアニメーションの一覧を読み込む
 *
 * @param bone_index ボーンのインデックス
 * @param cluster FbxCluster
 */
void FbxFileLoader::load_animations_for_bone( int bone_index, FbxCluster* cluster )
{
	if ( ! cluster )
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
		animation.set_name( cluster->GetName() );

		for ( int m = 0; m < anim_stack->GetMemberCount< FbxAnimLayer >(); m++ )
		{
			FbxAnimLayer* anim_layer = anim_stack->GetMember< FbxAnimLayer >( m );

			if ( ! anim_layer )
			{
				continue;
			}

			load_curve_for_animation( animation, "tx", cluster->GetLink()->LclTranslation.GetCurve( anim_layer, FBXSDK_CURVENODE_COMPONENT_X ) );
			load_curve_for_animation( animation, "ty", cluster->GetLink()->LclTranslation.GetCurve( anim_layer, FBXSDK_CURVENODE_COMPONENT_Y ) );
			load_curve_for_animation( animation, "tz", cluster->GetLink()->LclTranslation.GetCurve( anim_layer, FBXSDK_CURVENODE_COMPONENT_Z ) );

			load_curve_for_animation( animation, "rx", cluster->GetLink()->LclRotation.GetCurve( anim_layer, FBXSDK_CURVENODE_COMPONENT_X ) );
			load_curve_for_animation( animation, "ry", cluster->GetLink()->LclRotation.GetCurve( anim_layer, FBXSDK_CURVENODE_COMPONENT_Y ) );
			load_curve_for_animation( animation, "rz", cluster->GetLink()->LclRotation.GetCurve( anim_layer, FBXSDK_CURVENODE_COMPONENT_Z ) );

			load_curve_for_animation( animation, "sx", cluster->GetLink()->LclScaling.GetCurve( anim_layer, FBXSDK_CURVENODE_COMPONENT_X ) );
			load_curve_for_animation( animation, "sy", cluster->GetLink()->LclScaling.GetCurve( anim_layer, FBXSDK_CURVENODE_COMPONENT_Y ) );
			load_curve_for_animation( animation, "sz", cluster->GetLink()->LclScaling.GetCurve( anim_layer, FBXSDK_CURVENODE_COMPONENT_Z ) );

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

			const char* rotate_channel_names[] = { "rx", "ry", "rz" };

			for ( int o = 0; o < 3; ++o )
			{
				if ( animation.has_channel( rotate_channel_names[ o ]  ) )
				{
					std::for_each(
						animation.get_channel( rotate_channel_names[ o ] ).get_key_frame_list().begin(),
						animation.get_channel( rotate_channel_names[ o ] ).get_key_frame_list().end(),
						[] ( AnimationKeyFrame& key_frame ) { key_frame.get_value() = math::degree_to_radian( key_frame.get_value() ); } );
				}
			}
		}
	}
}

/**
 * 指定したアニメーションにアニメーションカーブを読み込む
 *
 * @param animation アニメーション
 * @param channel_name アニメーションのチャンネル名
 * @param anim_curve FbxAnimCurve
 */
void FbxFileLoader::load_curve_for_animation( Animation& animation, const char_t* channel_name, const FbxAnimCurve* anim_curve )
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

		animation.get_channel( channel_name ).add_key_frame( key_frame );
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
		std::swap( i->Position.y, i->Position.z );
		std::swap( i->Normal.y, i->Normal.z );
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