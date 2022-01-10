#include "GraphicsManager.h"

#include <ActiveObjectManager.h>

#include <blue_sky/graphics/Fader.h>
#include <blue_sky/graphics/Rectangle.h>
#include <blue_sky/graphics/ObjFileLoader.h>
#include <blue_sky/graphics/FbxFileLoader.h>
#include <blue_sky/graphics/Line.h>

/// @todo 別 cpp に移動
#include <blue_sky/graphics/shader/NullShader.h>
#include <blue_sky/graphics/shader/FlatShader.h>
#include <blue_sky/graphics/shader/FaderShader.h>
#include <blue_sky/graphics/shader/MatcapShader.h>
#include <blue_sky/graphics/shader/ShadowMapShader.h>
#include <blue_sky/graphics/shader/TessellationMatcapShader.h>
#include <blue_sky/graphics/shader/UnicolorShader.h>
#include <blue_sky/graphics/shader/DebugShadowMapTextureShader.h>
#include <blue_sky/graphics/shader/post_effect/DefaultShader.h>
#include <blue_sky/graphics/shader/post_effect/HandDrawingShader.h>

#include <core/animation/AnimationPlayer.h>
#include <core/graphics/ShadowMap.h>

#include <core/graphics/Shader.h>
#include <core/graphics/NullTexture.h>
#include <core/graphics/RenderTargetTexture.h>
#include <core/graphics/BackBufferTexture.h>

#include <common/timer.h>
#include <common/exception.h>

#include <boost/filesystem.hpp>

#include <iostream>

namespace blue_sky::graphics
{

GraphicsManager::GraphicsManager()
	: fader_( std::make_unique< Fader >() )
	, post_effect_rectangle_( std::make_unique< Model >() )
	, null_texture_( texture_manager_.create< core::graphics::NullTexture >() )
{
	
}

GraphicsManager::~GraphicsManager()
{

}


void GraphicsManager::setup_shadow_map( uint_t levels, uint_t size )
{
	shadow_map_.reset( create_shadow_map( levels, size ) );
		
	shadow_map_shader_->set_constant_buffer( shadow_map_->get_constant_buffer() );
	shadow_map_skin_shader_->set_constant_buffer( shadow_map_->get_constant_buffer() );
}

void GraphicsManager::unset_shadow_map()
{
	shadow_map_.reset();
}

const ActiveObjectManager* GraphicsManager::get_game_object_manager() const
{
	return GameMain::get_instance()->get_active_object_manager();
}

/**
 * 更新処理
 *
 */
void GraphicsManager::update()
{
	if ( is_fading_in_ )
	{
		fade_in( fade_speed_ );
	}
	else
	{
		fade_out( fade_speed_ );
	}
}

/**
 * 指定したモデルにファイルからメッシュを読み込む
 *
 * @param model モデル
 * @param name モデル名
 */
bool_t GraphicsManager::load_mesh( Model* model, const char_t* name )
{
	const auto file_path = string_t( "media/model/" ) + name;

	bool mesh_loaded = false;

	if ( boost::filesystem::exists( file_path + ".bin.fbx" ) )
	{
		if ( ! boost::filesystem::exists( file_path + ".fbx" ) || boost::filesystem::last_write_time( file_path + ".fbx" ) < boost::filesystem::last_write_time( file_path + ".bin.fbx" ) )
		{
			FbxFileLoader loader( model, name );
			mesh_loaded = loader.load( ( file_path + ".bin.fbx" ).c_str() );
		}
	}

	if ( ! mesh_loaded && boost::filesystem::exists( file_path  + ".fbx" ) )
	{
		FbxFileLoader loader( model, name );
		mesh_loaded = loader.load( ( file_path + ".fbx" ).c_str() );

		if ( mesh_loaded )
		{
			loader.save_fbx( ( file_path + ".bin.fbx" ).c_str() );
		}
	}

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

		for ( size_t n = 0; n < model->get_mesh()->get_rendering_vertex_group_count(); n++ )
		{
			if ( n >= model->get_shader_count() || ! model->get_shader_at( n ) )
			{
				model->set_shader_at( n, get_shader( "null" ) );
			}
		}
	}

	return mesh_loaded;
}

/**
 * 指定したモデルにファイルからラインを読み込む
 *
 * @param model モデル
 * @param name モデル名
 */
bool_t GraphicsManager::load_line( Model* model, const char_t* name )
{
	const auto line_file_path = string_t( "media/model/" ) + name + "-line.obj";

	if ( boost::filesystem::exists( line_file_path ) )
	{
		model->set_line( create_line() );
		return model->get_line()->load_obj( line_file_path.c_str() );
	}

	return false;
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

	model = new Model();

	common::timer t;

	std::cout << "--- loading : " << name << " ---" << std::endl;
	
	bool mesh_loaded = load_mesh( model, name );
	bool line_loaded = load_line( model, name );

	if ( mesh_loaded || line_loaded )
	{
		if ( mesh_loaded )
		{
			std::cout << "--- loaded mesh : " << name << " : " << t.elapsed() << "---" << std::endl;
		}
		if ( line_loaded )
		{
			std::cout << "--- loaded line : " << name << " : " << t.elapsed() << "---" << std::endl;
		}

		model_manager_.add_named( name, model );
	}
	else
	{
		std::cout << "--- not found : " << name << " : " << t.elapsed() << "---" << std::endl;

		delete model;

		return nullptr;
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
 * @param name テクスチャ名
 * @param file_name ファイルパス
 */
GraphicsManager::Texture* GraphicsManager::load_named_texture( const char_t* name, const char_t* file_path )
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

/**
 * 名前を指定してテクスチャをアンロードする
 *
 * @param name テクスチャ名
 */
void GraphicsManager::unload_named_texture( const char_t* name )
{
	texture_manager_.remove_named( name );
}

/**
 * デフォルトのシェーダーを準備する
 *
 * @todo スクリプトから行えるようにする ( C++ のシェーダークラスに名前を付けておき、スクリプトからシェーダークラス名, InputLayout 名, テクニック名を指定してシェーダーオブジェクトを new できないか？ )
 * @todo 関数名直す
 */
void GraphicsManager::setup_default_shaders()
{
	auto* matcap_texture = load_named_texture( "matcap", "media/texture/matcap/skin.png" );

	create_named_shader< shader::NullShader >( "null" );
	create_named_shader< shader::FlatShader >( "flat", "main", "flat" );
	create_named_shader< shader::FlatShader >( "flat_skin", "skin", "flat_skin" );
	create_named_shader< shader::MatcapShader >( "matcap", "main", "matcap" )->set_texture( matcap_texture );
	create_named_shader< shader::MatcapShader >( "matcap_skin", "skin", "matcap_skin" )->set_texture( matcap_texture );
	create_named_shader< shader::ShadowMapShader >( "shadow_map" );
	create_named_shader< shader::SkinningShadowMapShader >( "shadow_map_skin" );
	create_named_shader< shader::TessellationMatcapShader >( "tess_matcap" )->set_texture( matcap_texture );
	create_named_shader< shader::SkinningTessellationMatcapShader >( "tess_matcap_skin" )->set_texture( matcap_texture );
	create_named_shader< shader::UnicolorShader >( "unicolor", "main", "unicolor" );

	auto post_effect_shader = create_named_shader< shader::post_effect::DefaultShader >( "post_effect_default" );
	create_named_shader< shader::post_effect::DefaultShader >( "post_effect_chromatic_aberrration", "main", "post_effect_chromatic_aberrration" );
	create_named_shader< shader::post_effect::HandDrawingShader >( "post_effect_hand_drawing", "main", "post_effect_hand_drawing" );

	create_named_shader< shader::DebugShadowMapTextureShader >( "debug_shadow_map_texture" );

	shadow_map_shader_ = get_shader< graphics::shader::BaseShadowMapShader >( "shadow_map" );
	shadow_map_skin_shader_ = get_shader< graphics::shader::BaseShadowMapShader >( "shadow_map_skin" );

	auto rectangle = create_named_mesh< Rectangle >( "rectangle", Mesh::Buffer::Type::DEFAULT );

	fader_->set_mesh( rectangle );
	fader_->set_shader_at( 0, create_named_shader< shader::FaderShader >( "fader" ) );

	post_effect_rectangle_->set_mesh( rectangle );
	set_post_effect_shader( post_effect_shader );
}

/**
 * 紙テクスチャを読み込む
 *
 */
void GraphicsManager::load_paper_textures()
{
	paper_texture_list_.clear();

	paper_texture_list_.push_back( load_named_texture( "paper-0", "media/texture/pencil-face-1.png" ) );
	paper_texture_list_.push_back( load_named_texture( "paper-1", "media/texture/pen-face-1-loop.png" ) );
	paper_texture_list_.push_back( load_named_texture( "paper-2", "media/texture/pen-face-2-loop.png" ) );
	paper_texture_list_.push_back( load_named_texture( "paper-3", "media/texture/dot-face-1.png" ) );
	paper_texture_list_.push_back( load_named_texture( "paper-4", "media/texture/brush-face-1.png" ) );
	paper_texture_list_.push_back( load_named_texture( "paper-5", "media/texture/blank.png" ) );

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
 * 描画に必要なシェーダーリソースをアップデートする
 *
 */
void GraphicsManager::update_constant_buffers() const
{
	/// @todo 毎フレーム行う必要があるか？
	get_frame_drawing_render_data()->update();

	for ( const auto& game_object : get_game_object_manager()->active_object_list() )
	{
		if ( ! game_object->get_model() )
		{
			continue;
		}

		game_object->update_render_data();
	}
}

/**
 * シャドウマップを描画する
 *
 * @todo 最適化・高速化
 */
void GraphicsManager::render_shadow_map() const
{
	if ( ! shadow_map_ )
	{
		return;
	}

	shadow_map_->ready_to_render_shadow_map();

	render_shadow_map( shadow_map_shader_, false );
	render_shadow_map( shadow_map_skin_shader_, true );
}

/**
 * シャドウマップを描画する
 *
 * @param shader シェーダー
 * @param is_skin_mesh スキンメッシュフラグ
 */
void GraphicsManager::render_shadow_map( const BaseShadowMapShader* shader, bool is_skin_mesh ) const
{
	for ( int n = 0; n < shadow_map_->get_cascade_levels(); n++ )
	{
		shadow_map_->ready_to_render_shadow_map_with_cascade_level( n );

		get_frame_drawing_render_data()->bind_to_gs(); // for line

		for ( const auto* active_object : get_game_object_manager()->active_object_list() )
		{
			if ( ! active_object->get_model() )
			{
				continue;
			}

			if ( active_object->get_model()->is_skin_mesh() == is_skin_mesh )
			{
				set_current_object_constant_buffer( active_object->get_object_constant_buffer() );
				set_current_skinning_constant_buffer( active_object->get_animation_player() ? active_object->get_animation_player()->get_constant_buffer() : nullptr );

				active_object->render_mesh( shader );

				if ( active_object->get_model()->get_line() && active_object->get_model()->get_line()->is_cast_shadow() )
				{
					active_object->render_line();
				}
			}
		}
	}
}

/**
 * 全ての ActiveObject を描画する
 *
 */
void GraphicsManager::render_active_objects( const ActiveObjectManager* active_object_manager ) const
{
	if ( shadow_map_ )
	{
		shadow_map_->ready_to_render_scene();
	}

	for ( const auto& active_object : active_object_manager->active_object_list() )
	{
		if ( ! active_object->get_model() )
		{
			continue;
		}

		set_current_object_constant_buffer( active_object->get_object_constant_buffer() );
		set_current_skinning_constant_buffer( active_object->get_animation_player() ? active_object->get_animation_player()->get_constant_buffer() : nullptr );

		active_object->render_mesh();
	}

	set_input_layout( "line" );

	render_technique( "|drawing_line", [ this, active_object_manager ]
	{
		/// @todo 必要な定数バッファを必要なシェーダーにだけバインドするようにする
		get_game_render_data()->bind_to_all();
		get_frame_render_data()->bind_to_all();
		get_frame_drawing_render_data()->bind_to_all();

		for ( const auto* active_object : active_object_manager->active_object_list() )
		{
			if ( ! active_object->get_model() )
			{
				continue;
			}

			active_object->render_line();
		}
	} );
}

void GraphicsManager::set_render_target( RenderTarget* render_target )
{
	render_target->activate();
}

/**
 * ポストエフェクト用のシェーダを設定する
 *
 * @param shader ポストエフェクト用のシェーダー
 */
void GraphicsManager::set_post_effect_shader( Shader* shader )
{
	post_effect_rectangle_->set_shader_at( 0, shader );
}

/**
 * ポストエフェクトをレンダリングする
 *
 * @param t ポストエフェクト適用対象のレンダリング後テクスチャ
 */
void GraphicsManager::render_post_effect( RenderTargetTexture* t )
{
	set_render_target( get_back_buffer_texture() );

	post_effect_rectangle_->get_shader_at( 0 )->set_texture_at( 0, t );
	post_effect_rectangle_->render();
}

/**
 * フェードイン・フェードアウト用の色を設定する
 *
 * @param color 色
 */
void GraphicsManager::set_fade_color( const Color& color )
{
	get_fader()->set_color( color );
}

void GraphicsManager::start_fade_in( float_t speed )
{
	is_fading_in_ = true;
	fade_speed_ = speed;
}
	
void GraphicsManager::start_fade_out( float_t speed )
{
	is_fading_in_ = false;
	fade_speed_ = speed;
}

void GraphicsManager::fade_in( float_t speed )
{
	get_fader()->fade_in( speed );
}
	
void GraphicsManager::fade_out( float_t speed )
{
	get_fader()->fade_out( speed );
}

/**
 * フェーダーを描画する
 *
 */
void GraphicsManager::render_fader() const
{
	ObjectConstantBufferData shader_data;
	shader_data.world = Matrix().set_identity();
	shader_data.color = get_fader()->get_color();

	get_shared_object_render_data()->update( & shader_data );
	set_current_object_constant_buffer( get_shared_object_render_data() );

	get_fader()->render();

#if 0
	ObjectConstantBufferData buffer_data;
	buffer_data.world = Matrix().set_identity().transpose();
	buffer_data.color = direct_3d_->getFader()->get_color();
	
	get_shared_object_render_data()->update( & buffer_data );

	/// @todo 2D の描画に法線を使っているのは無駄なのでなんとかする
	set_input_layout( "main" );

	render_technique( "|main2d", [this]
	{
		get_shared_object_render_data()->bind_to_vs();
		get_shared_object_render_data()->bind_to_ps();

		direct_3d_->getFader()->render();
	} );
#endif
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

	core::AnimationPlayer::BoneConstantBufferData bone_constant_buffer_data;
	active_object->get_animation_player()->calculate_bone_matrix_recursive( bone_constant_buffer_data, 0, Matrix::identity() );

	for ( uint_t n = 0; n < active_object->get_animation_player()->get_skinning_animation_set()->get_bone_count(); ++n )
	{
		const Matrix& bone_offset_matrix = active_object->get_animation_player()->get_skinning_animation_set()->get_bone_offset_matrix_by_bone_index( n );
		const Matrix& bone_matrix = bone_offset_matrix * bone_constant_buffer_data.bone_matrix[ n ];
		// const Matrix& bone_matrix = bone_offset_matrix;

		const core::Transform& t = active_object->get_transform();

		ObjectConstantBufferData shader_data;

		shader_data.world = Matrix().set_scaling( 0.1f, 0.1f, 0.1f );
		shader_data.world *= bone_matrix;
		shader_data.world *= Matrix().set_rotation_quaternion( t.get_rotation() );
		shader_data.world *= Matrix().set_translation( t.get_position().x(), t.get_position().y(), t.get_position().z() );
		shader_data.world = shader_data.world.transpose();

		if ( active_object->get_model()->get_line() )
		{
			shader_data.color = active_object->get_model()->get_line()->get_color();
		}

		get_shared_object_render_data()->update( & shader_data );

		render_debug_axis_model();
	}
}

/**
 * デバッグのためにリソースの一覧を標準出力に出力する
 *
 */
void GraphicsManager::debug_print_resources() const
{
	std::cout << "model     : "; model_manager_.debug_print_resources();
	std::cout << "mesh      : "; mesh_manager_.debug_print_resources();
	std::cout << "shader    : "; shader_manager_.debug_print_resources();
	std::cout << "texture   : "; texture_manager_.debug_print_resources();
	std::cout << "skin_anim : "; skinning_animation_set_manager_.debug_print_resources();
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
