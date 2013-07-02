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
 * FBX �t�@�C����ǂݍ���
 *
 * @param file_name FBX �t�@�C����
 * @return �t�@�C���̓ǂݍ��݂ɐ��������ꍇ�� true ���A���s�����ꍇ�� false ��Ԃ�
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

		for ( int n = 0; n < root_node->GetChildCount(); n++ )
		{
			load_limb_recursive( root_node->GetChild( n ) );
		}

		load_animations();

		for ( int n = 0; n < root_node->GetChildCount(); n++ )
		{
			load_mesh_recursive( root_node->GetChild( n ) );
		}
	}

	convert_coordinate_system();

	return true;
}

/**
 * FbxNode ���烁�b�V�������ċA�I�ɓǂݍ���
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
 * ���b�V������ǂݍ���
 *
 * @param mesh ���b�V�����
 */
void FbxFileLoader::load_mesh( FbxMesh* mesh )
{
	if ( ! mesh )
	{
		return;
	}

	FbxGeometryConverter converter( fbx_manager_ );
	mesh = converter.TriangulateMesh( mesh );

	typedef std::map< Mesh::Vertex, Material::Index > VertexIndexMap;
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
		for ( int n = 0; n < mesh->GetLayerCount(); n++ )
		{
			FbxLayer* layer = mesh->GetLayer( n );
			smoothing = layer->GetSmoothing();

			break;
		}
	}

	assert( smoothing );

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
			// �|���S���̖@�����v�Z����
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
				
			if ( mesh->GetPolygonVertexUV( n, m, "UVMap", uv_vector ) )
			{
				v.TexCoord = Mesh::TexCoord(
					static_cast< float >( uv_vector[ 0 ] ),
					1.f - static_cast< float >( uv_vector[ 1 ] ) );
			}

			if ( is_smooth )
			{
				FbxVector4 normal_vector;

				// ���_�̖@��
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
				// �|���S���̖@��
				v.Normal = Mesh::Normal(
					static_cast< float >( polygon_normal[ 0 ] ),
					static_cast< float >( polygon_normal[ 1 ] ),
					static_cast< float >( polygon_normal[ 2 ] ) );
			}
			
			// ���_�̈ꗗ�ɒǉ�
			{
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
}

/**
 * ���b�V���̃X�L�j���O����ǂݍ���
 *
 * @param mesh ���b�V�����
 * @param skinning_info_list �X�L�j���O���ւ̎Q��
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

		Matrix rx, ry, rz, s, t;

		rx.set_rotation_x( math::degree_to_radian( static_cast< float_t >( -initial_matrix.GetR()[ 0 ] ) ) );
		ry.set_rotation_y( math::degree_to_radian( static_cast< float_t >( -initial_matrix.GetR()[ 1 ] ) ) );
		rz.set_rotation_z( math::degree_to_radian( static_cast< float_t >(  initial_matrix.GetR()[ 2 ] ) ) );

		s.set_scaling(
			static_cast< float_t >( initial_matrix.GetS()[ 0 ] ),
			static_cast< float_t >( initial_matrix.GetS()[ 1 ] ),
			static_cast< float_t >( initial_matrix.GetS()[ 2 ] ) );

		t.set_translation(
			static_cast< float_t >(  initial_matrix.GetT()[ 0 ] ),
			static_cast< float_t >(  initial_matrix.GetT()[ 1 ] ),
			static_cast< float_t >( -initial_matrix.GetT()[ 2 ] ) );

		mesh_->get_skinning_animation_set()->get_bone_offset_matrix_by_bone_index( bone_index ) = s * rx * ry * rz * t;
	}

	if ( mesh_->get_skinning_animation_set() )
	{
		mesh_->get_skinning_animation_set()->optimize();
		mesh_->get_skinning_animation_set()->calculate_length();
	}
}

/**
 * �}�e���A������ǂݍ���
 *
 * @param fbx_material �}�e���A�����
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
 * FbxNode ����{�[�������ċA�I�ɓǂݍ���
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
 * �X�P���g������ǂݍ���
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
 * �A�j���[�V��������ǂݍ���
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
 * �w�肵���C���f�b�N�X�̃{�[���ɑΉ�����A�j���[�V�����̈ꗗ��ǂݍ���
 *
 * @param bone_index �{�[���̃C���f�b�N�X
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

			load_curve_for_animation( animation, "tx", node->LclTranslation.GetCurve( anim_layer, FBXSDK_CURVENODE_COMPONENT_X ) );
			load_curve_for_animation( animation, "ty", node->LclTranslation.GetCurve( anim_layer, FBXSDK_CURVENODE_COMPONENT_Y ) );
			load_curve_for_animation( animation, "tz", node->LclTranslation.GetCurve( anim_layer, FBXSDK_CURVENODE_COMPONENT_Z ) );

			load_curve_for_animation( animation, "rx", node->LclRotation.GetCurve( anim_layer, FBXSDK_CURVENODE_COMPONENT_X ) );
			load_curve_for_animation( animation, "ry", node->LclRotation.GetCurve( anim_layer, FBXSDK_CURVENODE_COMPONENT_Y ) );
			load_curve_for_animation( animation, "rz", node->LclRotation.GetCurve( anim_layer, FBXSDK_CURVENODE_COMPONENT_Z ) );

			load_curve_for_animation( animation, "sx", node->LclScaling.GetCurve( anim_layer, FBXSDK_CURVENODE_COMPONENT_X ) );
			load_curve_for_animation( animation, "sy", node->LclScaling.GetCurve( anim_layer, FBXSDK_CURVENODE_COMPONENT_Y ) );
			load_curve_for_animation( animation, "sz", node->LclScaling.GetCurve( anim_layer, FBXSDK_CURVENODE_COMPONENT_Z ) );

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

			const char* inverse_channel_names[] = { "tz", "rx", "ry" };

			for ( int o = 0; o < 3; ++o )
			{
				if ( animation.has_channel( inverse_channel_names[ o ]  ) )
				{
					std::for_each(
						animation.get_channel( inverse_channel_names[ o ] ).get_key_frame_list().begin(),
						animation.get_channel( inverse_channel_names[ o ] ).get_key_frame_list().end(),
						[] ( AnimationKeyFrame& key_frame ) { key_frame.get_value() = -key_frame.get_value(); } );
				}
			}
		}
	}
}

/**
 * �w�肵���A�j���[�V�����ɃA�j���[�V�����J�[�u��ǂݍ���
 *
 * @param animation �A�j���[�V����
 * @param channel_name �A�j���[�V�����̃`�����l����
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
 * ���W�n��ϊ�����
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
 * ���W�n��\������
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