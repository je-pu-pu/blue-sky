#include "GraphicsManager.h"

#include <blue_sky/graphics/Fader.h>
#include <blue_sky/graphics/ObjFileLoader.h>
#include <blue_sky/graphics/FbxFileLoader.h>
#include <blue_sky/graphics/shader/FlatShader.h>
#include <blue_sky/graphics/shader/MatcapShader.h>
#include <blue_sky/graphics/Line.h>

#include <ActiveObjectManager.h>

#include <AnimationPlayer.h>

#include <game/Texture.h>

#include <common/timer.h>
#include <common/exception.h>

#include <boost/filesystem.hpp>

#include <iostream>

namespace blue_sky::graphics
{

GraphicsManager::GraphicsManager()
	: fader_( new Fader() )
{
	
}

GraphicsManager::~GraphicsManager()
{

}

/**
 * モデルを読み込む
 *
 * @param name 名前
 * @return モデル
 */
GraphicsManager::Model* GraphicsManager::load_model( const char_t* name )
{
	Model* model = model_manager_.get( name );
	
	if ( model )
	{
		return model;
	}

	model = create_named_model( name );
	// model->set_mesh( get_mesh( name ) );

	string_t file_path = string_t( "media/model/" ) + name;

	std::cout << "--- loading " << name << " ---" << std::endl;
	common::timer t;

	bool mesh_loaded = false;

	// Mesh
	if ( boost::filesystem::exists( file_path + ".bin.fbx" ) )
	{
		if ( ! boost::filesystem::exists( file_path + ".fbx" ) || boost::filesystem::last_write_time( file_path + ".fbx" ) < boost::filesystem::last_write_time( file_path + ".bin.fbx" ) )
		{
			FbxFileLoader loader( model, name );
			mesh_loaded = loader.load( ( file_path + ".bin.fbx" ).c_str() );
		}
	}

	/*
	if ( ! mesh_loaded && boost::filesystem::exists( file_path  + ".fbx" ) )
	{
		mesh_loaded = mesh->load_fbx( ( file_path  + ".fbx" ).c_str(), fbx_file_loader_.get(), skinning_animation_set );

		if ( loaded )
		{
			fbx_file_loader_->save( ( file_path + ".bin.fbx" ).c_str() );
		}
	}
	*/

	if ( ! mesh_loaded && boost::filesystem::exists( file_path + ".obj" ) )
	{
		ObjFileLoader loader( model, name );
		mesh_loaded = loader.load( ( file_path + ".obj" ).c_str() );
	}

	if ( mesh_loaded )
	{
		model->get_mesh()->optimize();

		model->get_mesh()->create_vertex_buffer();
		model->get_mesh()->create_index_buffer();

		model->get_mesh()->clear_vertex_list();
		model->get_mesh()->clear_vertex_weight_list();
		model->get_mesh()->clear_vertex_group_list();
	}

	// Line
	string_t line_file_path = string_t( "media/model/" ) + name + "-line.obj";
	bool line_loaded = false;

	if ( boost::filesystem::exists( line_file_path ) )
	{
		model->set_line( create_line() );
		line_loaded = model->get_line()->load_obj( line_file_path.c_str() );
	}

	if ( mesh_loaded || line_loaded )
	{
		std::cout << "--- loaded " << name << " : " << t.elapsed() << "---" << std::endl;
	}
	else
	{
		std::cout << "--- not found " << name << " : " << t.elapsed() << "---" << std::endl;
	}

	return model;
}

/**
 * モデルを複製する
 *
 */
GraphicsManager::Model* GraphicsManager::clone_model( const Model* m )
{
	return model_manager_.create( *m );
}

/**
 * 名前とファイル名を指定してテクスチャをファイルから読み込む
 *
 * 指定された名前のテクスチャがすでに管理されている場合は、テクスチャをファイルから読み込まず管理されているテクスチャを返す
 * 指定された名前のテクスチャが管理されていない場合は、指定されたファイルパスからテクスチャを読み込み、名前を付けて管理し、そのテクスチャを返す
 *
 * @param name 名前
 * @param file_name ファイルパス
 */
GraphicsManager::Texture* GraphicsManager::load_texture( const char_t* name, const char_t* file_path )
{
	Texture* t = texture_manager_.get( name );

	if ( t )
	{
		return t;
	}

	t = load_texture_file( file_path );

	if ( ! t )
	{
		return nullptr;
	}

	texture_manager_.add_named( name, t );

	return t;
}

/**
 * 名前を指定してテクスチャを取得する
 *
 * 指定された名前のテクスチャがすで管理されている場合は、そのテクスチャを返す
 * 指定された名前のテクスチャが管理されていない場合は、一定のルールに基づき名前をファイルパスに変換し、テクスチャをファイルから読み込み、名前で監視し、そのテクスチャを返す
 */
GraphicsManager::Texture* GraphicsManager::get_texture( const char_t* name )
{
	Texture* t = texture_manager_.get( name );

	if ( t )
	{
		return t;
	}

	string_t file_path = string_t( "media/model/" ) + name + ".png";

	t = load_texture_file( file_path.c_str() );

	if ( ! t )
	{
		return nullptr;
	}

	texture_manager_.add_named( name, t );

	return t;
}

#if 0
/**
 * 手描き風ラインを読み込む
 *
 * @param name 名前
 * @return 手描き風ライン、または失敗時に 0 を返す
 */
DrawingLine* GraphicsManager::load_drawing_line( const char_t* name )
{
	string_t file_path = string_t( "media/model/" ) + name + "-line.obj";

	if ( ! boost::filesystem::exists( file_path ) )
	{
		return 0;
	}

	DrawingLine* line = create_drawing_line();
	line->load_obj( file_path.c_str() );

	return line;
}
#endif

/**
 * デフォルトのシェーダーを準備する
 *
 */
void GraphicsManager::setup_default_shaders()
{
	auto* matcap_texture = load_texture( "matcap", "media/texture/matcap/skin.png" );

	create_named_shader< shader::FlatShader >( "flat", "main", "flat" );
	create_named_shader< shader::FlatShader >( "flat_skin", "skin", "flat_skin" );
	create_named_shader< shader::MatcapShader >( "matcap", "main", "matcap" )->set_texture( matcap_texture );
	create_named_shader< shader::MatcapShader >( "matcap_skin", "skin", "matcap_skin" )->set_texture( matcap_texture );
}

/**
 * 紙テクスチャを読み込む
 *
 */
void GraphicsManager::load_paper_textures()
{
	paper_texture_list_.clear();

	paper_texture_list_.push_back( load_texture( "paper-0", "media/texture/pencil-face-1.png" ) );
	paper_texture_list_.push_back( load_texture( "paper-1", "media/texture/pen-face-1-loop.png" ) );
	paper_texture_list_.push_back( load_texture( "paper-2", "media/texture/pen-face-2-loop.png" ) );
	paper_texture_list_.push_back( load_texture( "paper-3", "media/texture/dot-face-1.png" ) );
	paper_texture_list_.push_back( load_texture( "paper-4", "media/texture/brush-face-1.png" ) );
	paper_texture_list_.push_back( load_texture( "paper-5", "media/texture/blank.png" ) );

	paper_texture_ = paper_texture_list_.back();
}

/**
 * 紙テクスチャのタイプを設定する
 *
 * @param type 紙テクスチャのタイプ
 */
void GraphicsManager::set_paper_texture_type( int_t type )
{
	if ( paper_texture_list_.size() > 0 )
	{
		type = math::clamp< int >( type, 0, paper_texture_list_.size() - 1 );
		paper_texture_ = paper_texture_list_[ type ];
	}
	else
	{
		paper_texture_ = 0;
	}
}

/**
 * 紙テクスチャをシェーダーにバインドする
 *
 * @todo unload_texture(), unload_texture_all() でテクスチャがアンロードされた場合に正しく対応する
 */
void GraphicsManager::bind_paper_texture() const
{
	if ( paper_texture_ )
	{
		paper_texture_->bind_to_ps( 2 );
	}
}

/**
 * 全ての ActiveObject を描画する
 *
 * @todo シェーダーを切り替えられるように整理する
 * @todo shadow_map_, paper_texture_ に対応し GamePlayScene の render_object_mesh(), render_object_line() を置き換える
 */
void GraphicsManager::render_active_objects( const ActiveObjectManager* active_object_manager ) const
{
	for ( const auto& active_object : active_object_manager->active_object_list() )
	{
		current_object_shader_resource_ = active_object->get_object_constant_buffer();
		current_skinning_shader_resource_ = active_object->get_animation_player() ? active_object->get_animation_player()->get_constant_buffer() : nullptr;

		active_object->update_render_data();
		active_object->render_mesh();
	}

#if 0
	set_input_layout( "main" );

	render_technique( "|flat", [ this, active_object_manager ]
	{
		

		bind_paper_texture();

		for ( const auto* active_object : active_object_manager->active_object_list() )
		{
			if ( ! active_object->get_drawing_model()->is_skin_mesh() )
			{
				active_object->render_mesh();
			}
		}
	} );

	set_input_layout( "skin" );

	render_technique( "|flat_skin", [ this, active_object_manager ]
	{
		/*
		get_game_render_data()->bind_to_vs();
		get_game_render_data()->bind_to_gs();
		
		get_frame_render_data()->bind_to_vs();

		get_frame_drawing_render_data()->bind_to_gs();
		*/

		get_game_render_data()->bind_to_all();
		get_frame_render_data()->bind_to_all();
		get_frame_drawing_render_data()->bind_to_all();

		bind_paper_texture();

		/*

		if ( shadow_map_ )
		{
			shadow_map_->ready_to_render_scene();
		}
		*/

		for ( const auto* active_object : active_object_manager->active_object_list() )
		{
			if ( active_object->get_drawing_model()->is_skin_mesh() )
			{
				active_object->render_mesh();
			}
		}
	} );

	// setup_rendering();
#endif

	set_input_layout( "line" );

	render_technique( "|drawing_line", [ this, active_object_manager ]
	{
		/// @todo 必要な定数バッファを必要なシェーダーにだけバインドするようにする
		get_game_render_data()->bind_to_all();
		get_frame_render_data()->bind_to_all();
		get_frame_drawing_render_data()->bind_to_all();

		for ( const auto* active_object : active_object_manager->active_object_list() )
		{
			active_object->render_line();
		}
	} );
}

/**
 * デバッグ用の軸を表示する
 *
 */
void GraphicsManager::render_debug_axis( const ActiveObjectManager* active_object_manager ) const
{
	if ( ! is_debug_axis_enabled_ )
	{
		return;
	}

	render_technique( "|simple_line", [ this, active_object_manager ]
	{
		get_frame_render_data()->bind_to_all();

		for ( const auto* active_object : active_object_manager->active_object_list() )
		{
			active_object->bind_render_data();

			render_debug_axis_model();

			render_debug_axis_for_bones( active_object );
		}
	} );
}

/**
 * ActiveObject の全ボーンに対してデバッグ用の軸を表示する
 *
 */
void GraphicsManager::render_debug_axis_for_bones( const ActiveObject* active_object ) const
{
	if ( active_object->is_dead() )
	{
		return;
	}

	if ( ! active_object->get_animation_player() )
	{
		return;
	}
	
	get_shared_object_render_data()->bind_to_vs();
	get_shared_object_render_data()->bind_to_ps();

	AnimationPlayer::BoneConstantBuffer::Data bone_constant_buffer_data;
	active_object->get_animation_player()->calculate_bone_matrix_recursive( bone_constant_buffer_data, 0, Matrix::identity() );

	for ( uint_t n = 0; n < active_object->get_animation_player()->get_skinning_animation_set()->get_bone_count(); ++n )
	{
		const Matrix& bone_offset_matrix = active_object->get_animation_player()->get_skinning_animation_set()->get_bone_offset_matrix_by_bone_index( n );
		const Matrix& bone_matrix = bone_offset_matrix * bone_constant_buffer_data.bone_matrix[ n ];
		// const Matrix& bone_matrix = bone_offset_matrix;

		const btTransform& trans = active_object->get_transform();

		Vector q( trans.getRotation().x(), trans.getRotation().y(), trans.getRotation().z(), trans.getRotation().w() );

		ObjectConstantBufferData buffer_data;

		buffer_data.world = Matrix().set_scaling( 0.1f, 0.1f, 0.1f );
		buffer_data.world *= bone_matrix;
		buffer_data.world *= Matrix().set_rotation_quaternion( q );
		buffer_data.world *= Matrix().set_translation( trans.getOrigin().x(), trans.getOrigin().y(), trans.getOrigin().z() );
		buffer_data.world = buffer_data.world.transpose();

		if ( active_object->get_model()->get_line() )
		{
			buffer_data.color = active_object->get_model()->get_line()->get_color();
		}

		get_shared_object_render_data()->update( & buffer_data );

		render_debug_axis_model();
	}
}

/*
void GraphicsManager::update_frame_render_data( const Camera* camera_ )
{
	FrameConstantBufferData frame_constant_buffer_data;
	update_frame_constant_buffer_data_sub( frame_constant_buffer_data );

	Vector eye( camera_->position().x(), camera_->position().y(), camera_->position().z() );
	Vector at( camera_->look_at().x(), camera_->look_at().y(), camera_->look_at().z() );
	Vector up( camera_->up().x(), camera_->up().y(), camera_->up().z() );

	frame_constant_buffer_data.view = ( Matrix().set_look_at( eye, at, up ) ).transpose();
	frame_constant_buffer_data.projection = Matrix().set_perspective_fov( math::degree_to_radian( camera_->fov() ), camera_->aspect(), camera_->near_clip(), camera_->far_clip() ).transpose();

	get_frame_render_data()->update( & frame_constant_buffer_data );
}
*/

} // namespace blue_sky::graphics
