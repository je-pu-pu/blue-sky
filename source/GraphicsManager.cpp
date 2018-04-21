#include "GraphicsManager.h"
#include "DrawingMesh.h"
#include "DrawingLine.h"
#include "FbxFileLoader.h"

#include <DrawingModel.h>
#include <ActiveObjectManager.h>

#include <common/timer.h>
#include <common/exception.h>

#include <boost/filesystem.hpp>

#include <iostream>

namespace blue_sky
{

GraphicsManager::GraphicsManager()
	: fbx_file_loader_( new FbxFileLoader() )
{
	
}

GraphicsManager::~GraphicsManager()
{

}

/**
 * 手描き風メッシュを読み込む
 *
 * @param name 名前
 * @param skinning_animation_set 同時に読み込むスキニングアニメーション情報を格納するポインタ
 * @return 手描き風メッシュ、または失敗時に 0 を返す
 */
DrawingMesh* GraphicsManager::load_drawing_mesh( const char_t* name, common::safe_ptr< SkinningAnimationSet >& skinning_animation_set )
{
	std::string file_path = string_t( "media/model/" ) + name;

	std::cout << "--- loading " << name << " ---" << std::endl;
	common::timer t;

	DrawingMesh* mesh = create_drawing_mesh();
	
	bool loaded = false;

	if ( ! loaded && boost::filesystem::exists( file_path + ".bin.fbx" ) )
	{
		if ( ! boost::filesystem::exists( file_path + ".fbx" ) || boost::filesystem::last_write_time( file_path + ".fbx" ) < boost::filesystem::last_write_time( file_path + ".bin.fbx" ) )
		{
			loaded = mesh->load_fbx( ( file_path + ".bin.fbx" ).c_str(), fbx_file_loader_.get(), skinning_animation_set );
		}
	}

	/*
	if ( ! loaded && boost::filesystem::exists( file_path  + ".fbx" ) )
	{
		loaded = mesh->load_fbx( ( file_path  + ".fbx" ).c_str(), fbx_file_loader_.get(), skinning_animation_set );

		if ( loaded )
		{
			fbx_file_loader_->save( ( file_path + ".bin.fbx" ).c_str() );
		}
	}
	*/

	if ( ! loaded && boost::filesystem::exists( file_path + ".obj" ) )
	{
		loaded = mesh->load_obj( ( file_path + ".obj" ).c_str() );
	}

	if ( loaded )
	{
		std::cout << "--- loaded " << name << " : " << t.elapsed() << "---" << std::endl;
	}
	else
	{
		std::cout << "--- not found " << name << " : " << t.elapsed() << "---" << std::endl;
	}

	

	return mesh;
}

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

/**
 * 全ての ActiveObject を描画する
 *
 * @todo shadow_map_, paper_texture_ に対応し GamePlayScene の render_object_mesh(), render_object_line() を置き換える
 */
void GraphicsManager::render_active_objects( const ActiveObjectManager* active_object_manager ) const
{
	for ( const auto& active_object : active_object_manager->active_object_list() )
	{
		active_object->update_render_data();
	}

	set_input_layout( "main" );

	render_technique( "|main_flat", [ this, active_object_manager ]
	{
		/// @todo 必要な定数バッファを必要なシェーダーにだけバインドするようにする
		get_game_render_data()->bind_to_all();
		get_frame_render_data()->bind_to_all();
		get_frame_drawing_render_data()->bind_to_all();

		for ( const auto* active_object : active_object_manager->active_object_list() )
		{
			if ( ! active_object->get_drawing_model()->is_skin_mesh() )
			{
				active_object->render_mesh();
			}
		}
	} );

	set_input_layout( "skin" );

	render_technique( "|skin_flat", [ this, active_object_manager ]
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

		/*
		paper_texture_->bind_to_ps( 2 );

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

}