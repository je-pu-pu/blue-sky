#include "GraphicsManager.h"

#include <blue_sky/GameMain.h>
#include <blue_sky/ecs/system/ObjectRenderDataSystem.h>
#include <blue_sky/ecs/component/AnimationComponent.h>

#include <blue_sky/graphics/Fader.h>
#include <blue_sky/graphics/Rectangle.h>
#include <blue_sky/graphics/ObjFileLoader.h>
#include <blue_sky/graphics/FbxFileLoader.h>
#include <blue_sky/graphics/Line.h>

#include <blue_sky/graphics/shader/AllShaders.h>

#include <Scene/Scene.h>

#include <core/animation/AnimationPlayer.h>
#include <core/graphics/ShadowMap.h>
#include <core/graphics/Frustum.h>

#include <core/graphics/Shader.h>
#include <core/graphics/NullTexture.h>
#include <core/graphics/RenderTargetTexture.h>
#include <core/graphics/BackBufferTexture.h>

#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/component/CameraComponent.h>
#include <core/ecs/EntityManager.h>
#include <core/TimeManager.h>

#include <common/timer.h>
#include <common/exception.h>

#include <boost/filesystem.hpp>

#include <algorithm>
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

ecs::ObjectRenderDataSystem* GraphicsManager::get_object_render_data_system() const
{
	return entity_manager_ ? entity_manager_->get_system< ecs::ObjectRenderDataSystem >() : nullptr;
}

/**
 * 物体の定数バッファ（world など）をシェーダーに設定する
 */
void GraphicsManager::bind_object_render_data( const ecs::ObjectRenderComponent& render ) const
{
	render.object_constant_buffer->bind_to_vs();
	render.object_constant_buffer->bind_to_ds();
	render.object_constant_buffer->bind_to_ps();
}

/**
 * 物体のメッシュを描く（描かない物体・メッシュの無いモデルは何もしない）
 */
void GraphicsManager::render_object_mesh( const ecs::ObjectRenderComponent& render, const Model* model ) const
{
	if ( ! render.should_draw_mesh() || ! model->get_mesh() )
	{
		return;
	}

	bind_object_render_data( render );

	if ( render.bone_constant_buffer )
	{
		render.bone_constant_buffer->bind_to_vs();
	}

	model->render();
}

/**
 * 物体のメッシュを、指定したシェーダーで描く（シャドウマップ用）
 */
void GraphicsManager::render_object_mesh( const ecs::ObjectRenderComponent& render, const Model* model, const Shader* shader ) const
{
	if ( ! render.should_draw_mesh() || ! model->get_mesh() )
	{
		return;
	}

	if ( render.bone_constant_buffer )
	{
		render.bone_constant_buffer->bind_to_vs();
	}

	shader->render_model( model );
}

/**
 * 物体の線を描く（描かない物体・線の無いモデルは何もしない）
 */
void GraphicsManager::render_object_line( const ecs::ObjectRenderComponent& render, const Model* model ) const
{
	if ( ! render.should_draw_line() || ! model->get_line() )
	{
		return;
	}

	bind_object_render_data( render );

	model->get_line()->render();
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
			FbxFileLoader loader( this, model, name );
			mesh_loaded = loader.load( ( file_path + ".bin.fbx" ).c_str() );
		}
	}

	if ( ! mesh_loaded && boost::filesystem::exists( file_path  + ".fbx" ) )
	{
		FbxFileLoader loader( this, model, name );
		mesh_loaded = loader.load( ( file_path + ".fbx" ).c_str() );

		/// Blender 5 系でエクスポートした FBX はもともとバイナリ形式なので変換しない
#if false
		if ( mesh_loaded )
		{
			loader.save_fbx( ( file_path + ".bin.fbx" ).c_str() );
		}
#endif
	}

	if ( ! mesh_loaded && boost::filesystem::exists( file_path + ".obj" ) )
	{
		ObjFileLoader loader( this, model, name );
		mesh_loaded = loader.load( ( file_path + ".obj" ).c_str() );
	}

	if ( mesh_loaded )
	{
		model->get_mesh()->optimize();

		model->get_mesh()->create_vertex_buffer();
		model->get_mesh()->create_index_buffer();

		// 頂点を囲む箱を覚えてから頂点を消す（見えない物を描かないために使う）
		model->get_mesh()->calculate_bounds();

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
	Model* model = model_manager_.get< Model >( name );

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
			std::cout << "--- loaded mesh : " << name << " : " << t.format() << "---" << std::endl;
		}
		if ( line_loaded )
		{
			std::cout << "--- loaded line : " << name << " : " << t.format() << "---" << std::endl;
		}

		model_manager_.add_named( name, model );
	}
	else
	{
		std::cout << "--- not found : " << name << " : " << t.format() << "---" << std::endl;

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
	return model_manager_.create< Model >( *m );
}

/**
 * モデルとシェーダーを複製する（メッシュ・線・アニメーションは共有する）
 *
 * 同じシェーダーが複数の頂点グループに割り当てられていれば、複製したものも 1 つを共有する（同じ割り当てのまま複製する）
 */
GraphicsManager::Model* GraphicsManager::clone_model_with_shaders( const Model* m )
{
	Model* model = clone_model( m );

	std::vector< std::pair< const Shader*, Shader* > > cloned;

	for ( size_t n = 0; n < model->get_shader_count(); n++ )
	{
		const Shader* original = model->get_shader_at( n );

		if ( ! original )
		{
			continue;
		}

		auto i = std::find_if( cloned.begin(), cloned.end(), [original] ( const auto& p ) { return p.first == original; } );

		if ( i == cloned.end() )
		{
			cloned.emplace_back( original, clone_shader( original ) );
			i = std::prev( cloned.end() );
		}

		model->set_shader_at( n, i->second );
	}

	return model;
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

void GraphicsManager::load_named_texture_premultiplied_async( const char_t* name, const char_t* file_path, std::function< void( Texture* ) > on_loaded )
{
	if ( Texture* t = texture_manager_.get( name ) )
	{
		on_loaded( t );
		return;
	}

	const string_t texture_name = name;

	start_texture_load_async( file_path, [this, texture_name, on_loaded] ( Texture* created )
	{
		if ( ! created )
		{
			on_loaded( nullptr );
			return;
		}

		// 読み込んでいる間に、同じ名前が同期の読み込みで登録されていたら、そちらを使う（作ったものは所有だけする）
		if ( Texture* existing = texture_manager_.get( texture_name ) )
		{
			texture_manager_.add( created );
			on_loaded( existing );
			return;
		}

		texture_manager_.add_named( texture_name, created );
		on_loaded( created );
	} );
}

GraphicsManager::Texture* GraphicsManager::load_named_texture_premultiplied( const char_t* name, const char_t* file_path )
{
	if ( Texture* t = texture_manager_.get( name ) )
	{
		return t;
	}

	Texture* t = load_texture_file_premultiplied( file_path );
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
 * 既定の描画の準備をする
 *
 * 以前はこの関数に 3 つの処理（シェーダーを作る・シャドウマップ用のシェーダーを選ぶ・画面全体の矩形を作る）が混ざっていた。
 * 呼ぶ順に意味がある（後の 2 つは、名前付きのシェーダーが作られていることを前提にする）
 */
void GraphicsManager::setup_default_shaders()
{
	create_default_shaders();
	select_shadow_map_shaders();
	setup_screen_rectangles();
}

/**
 * 既定の名前付きシェーダーを作る
 *
 * スクリプトから作れるようにしたいときは、シェーダーの型に名前を付けて登録し、型名・入力レイアウト名・テクニック名を受け取って作る関数をスクリプトに渡す
 */
void GraphicsManager::create_default_shaders()
{
	auto* matcap_texture = load_named_texture( "matcap", "media/texture/matcap/skin.png" );

	create_named_shader< shader::NullShader >( "null" );
	create_named_shader< shader::FlatShader >( "flat", "main", "flat" );
	create_named_shader< shader::FlatShader >( "flat_skin", "skin", "flat_skin" );
	create_named_shader< shader::LitShader >( "lit", "main", "lit" );
	create_named_shader< shader::LitInstancedShader >( "lit_instanced", "main", "lit_instanced" );
	create_named_shader< shader::MatcapShader >( "matcap", "main", "matcap" )->set_texture( matcap_texture );
	create_named_shader< shader::MatcapShader >( "matcap_skin", "skin", "matcap_skin" )->set_texture( matcap_texture );
	create_named_shader< shader::ShadowMapShader >( "shadow_map" );
	create_named_shader< shader::SkinningShadowMapShader >( "shadow_map_skin" );
	create_named_shader< shader::TessellationMatcapShader >( "tess_matcap" )->set_texture( matcap_texture );
	create_named_shader< shader::SkinningTessellationMatcapShader >( "tess_matcap_skin" )->set_texture( matcap_texture );
	create_named_shader< shader::UnicolorShader >( "unicolor", "main", "unicolor" );
	create_named_shader< shader::UnicolorShader >( "lit_unicolor", "main", "lit_unicolor" );	// 単色＋面陰影
	create_named_shader< shader::VelocityShader >( "velocity", "main", "velocity" );	// モーションベクトル ( 案B / 段階0b )

	// ポストエフェクトは 2D の入力レイアウト（法線なし）で描く
	create_named_shader< shader::post_effect::DefaultShader >( "post_effect_default" );
	create_named_shader< shader::post_effect::DefaultShader >( "post_effect_chromatic_aberration", "2d", "post_effect_chromatic_aberration" );
	create_named_shader< shader::post_effect::HandDrawingShader >( "post_effect_hand_drawing", "2d", "post_effect_hand_drawing" );
	create_named_shader< shader::post_effect::RecolorShader >( "post_effect_recolor", "2d", "post_effect_recolor" );

	create_named_shader< shader::post_effect::DefaultShader >( "post_effect_id_to_color", "2d", "post_effect_id_to_color" );
	create_named_shader< shader::post_effect::NoiseShader >( "post_effect_noise", "2d", "post_effect_noise" );
	create_named_shader< shader::post_effect::DefaultShader >( "post_effect_beat_pulse", "2d", "post_effect_beat_pulse" );

	create_named_shader< shader::DebugShadowMapTextureShader >( "debug_shadow_map_texture" );

	create_named_shader< shader::FaderShader >( "fader" );
}

/**
 * シャドウマップを描くシェーダーを選ぶ（create_default_shaders で作ったものから）
 */
void GraphicsManager::select_shadow_map_shaders()
{
	shadow_map_shader_ = get_shader< graphics::shader::BaseShadowMapShader >( "shadow_map" );
	shadow_map_skin_shader_ = get_shader< graphics::shader::BaseShadowMapShader >( "shadow_map_skin" );
}

/**
 * 画面全体を覆う矩形（フェーダーとポストエフェクト用）を作り、シェーダーを割り当てる
 */
void GraphicsManager::setup_screen_rectangles()
{
	auto rectangle = create_named_mesh< Rectangle >( "rectangle", Mesh::Buffer::Type::DEFAULT );

	fader_->set_mesh( rectangle );
	fader_->set_shader_at( 0, get_shader( "fader" ) );

	post_effect_rectangle_->set_mesh( rectangle );
	set_post_effect_shader( get_shader( "post_effect_default" ) );
}

/**
 * 全てのシェーダーを作成しなおす
 * 
 */
void GraphicsManager::refresh_all_shaders()
{
	// 全てのシェーダーを再読み込みする
	for ( auto& s: get_shader_manager().get_resource_list() )
	{
		s->reload();
	}
	
#if 0
	// 全てのシェーダーを削除する
	get_shader_manager().clear_all();

	// デフォルトのシェーダーを作成する
	setup_default_shaders();

	// （以前はここで全ての GameObject のシェーダーを作り直していた）
#endif
}

/**
 * 紙テクスチャを読み込む
 *
 */
void GraphicsManager::load_paper_textures()
{
	paper_texture_list_.clear();

	paper_texture_list_.push_back( texture_manager_.handle_of( load_named_texture( "paper-0", "media/texture/pencil-face-1.png" ) ) );
	paper_texture_list_.push_back( texture_manager_.handle_of( load_named_texture( "paper-1", "media/texture/pen-face-1-loop.png" ) ) );
	paper_texture_list_.push_back( texture_manager_.handle_of( load_named_texture( "paper-2", "media/texture/pen-face-2-loop.png" ) ) );
	paper_texture_list_.push_back( texture_manager_.handle_of( load_named_texture( "paper-3", "media/texture/dot-face-1.png" ) ) );
	paper_texture_list_.push_back( texture_manager_.handle_of( load_named_texture( "paper-4", "media/texture/brush-face-1.png" ) ) );
	paper_texture_list_.push_back( texture_manager_.handle_of( load_named_texture( "paper-5", "media/texture/blank.png" ) ) );

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
		paper_texture_ = TextureManager::WeakHandle();
	}
}

std::size_t GraphicsManager::flush_releases()
{
	// 描く対象のこのフレームのモデルを忘れる（次のフレームで引き直す。解放するモデルを指したまま持ち越さない）
	if ( auto* system = get_object_render_data_system() )
	{
		system->forget_resolved_models();
	}

	// + でつなぐと呼ぶ順が決まらないので、1 つずつ呼ぶ
	std::size_t count = model_manager_.flush_releases();
	count += mesh_manager_.flush_releases();
	count += get_shader_manager().flush_releases();
	count += texture_manager_.flush_releases();
	count += skinning_animation_set_manager_.flush_releases();

	return count;
}

/**
 * 紙テクスチャをシェーダーにバインドする（アンロード済みなら何もしない）
 */
void GraphicsManager::bind_paper_texture() const
{
	if ( Texture* t = paper_texture_.get() )
	{
		t->bind_to_ps( 2 );
	}
}

/**
 * 描画に必要なシェーダーリソースをアップデートする
 *
 */
void GraphicsManager::update_constant_buffers() const
{
	// 描画のアクセントなどは毎フレーム変わりうるので毎回更新する（1 フレームに 1 回の小さな定数バッファ）
	get_frame_drawing_render_data()->update();

	// ゲームの物体の定数（world・前フレームの world・線の色・ボーン行列）。以前は ActiveObject を回して ActiveObject::update_render_data を呼んでいた
	if ( auto* system = get_object_render_data_system() )
	{
		// 同じモデルの物体をまとめて描く（入れ物を作れたときだけ。作れなければ今までどおり 1 個ずつ描く）
		if ( ! object_instance_buffer_created_ )
		{
			object_instance_buffer_ = create_object_instance_buffer();
			object_instance_buffer_created_ = true;
		}

		system->set_instancing_enabled( static_cast< bool >( object_instance_buffer_ ) );
		system->update_render_data();

		// 物体ごとの定数を、まとめて描くときの入れ物に写す（1 フレームに 1 回。どの物を描くかの番号は、描く段ごとに写す）
		if ( system->is_instancing_ready() )
		{
			object_instance_buffer_->update_instances( system->get_instances().data(), system->get_instances().size() );
		}
	}
}

const core::graphics::ObjectInstanceBuffer* GraphicsManager::get_ready_object_instance_buffer( const ecs::ObjectRenderDataSystem* system ) const
{
	return system && system->is_instancing_ready() ? object_instance_buffer_.get() : nullptr;
}

/**
 * 描画に必要な準備を行う
 * 
 */
void GraphicsManager::setup_rendering()
{
	// メインカメラの情報が設定されていない場合は何もしない
	if ( ! main_camera_transform_component_ || ! main_camera_component_ )
	{
		return;
	}

	const auto& camera_transform = main_camera_transform_component_->transform;
	const auto* camera_component = main_camera_component_;

	const auto eye = camera_transform.get_position() + camera_component->eye_offset;
	const auto at = eye + camera_transform.forward();
	const auto up = camera_transform.up();

	const auto aspect = static_cast< float >( get_screen_width() ) / static_cast< float >( get_screen_height() );

	get_frame_render_data()->data().view = ( Matrix().set_look_at( eye, at, up ) );
	get_frame_render_data()->data().projection = Matrix().set_perspective_fov( math::degree_to_radian( camera_component->fov ), aspect, camera_component->near_clip, camera_component->far_clip );
	
	// 光の向きはシーンが set_light_direction で変えられる（既定は以前の固定値と同じ）
	get_frame_render_data()->data().light = light_direction_.get_normalized();

	// 経過時間（シェーダーの揺らぎに使う）も、カメラと同じフレームの定数に入れる
	get_frame_render_data()->data().time = time_manager_ ? time_manager_->get_total_elapsed_time() : 0.f;

	get_frame_render_data()->update();

	set_eye_position( eye );
}

void GraphicsManager::set_object_culling( const Matrix& camera_view_projection ) const
{
	auto* system = get_object_render_data_system();

	if ( ! system )
	{
		return;
	}

	core::graphics::Frustum shadow_frustums[ ecs::ObjectRenderDataSystem::MaxShadowCascades ];
	int cascade_count = 0;

	// 影を描くなら、カスケードごとの範囲（描くときにシェーダーへ渡すのと同じ行列）。求められなければ影はカリングしない
	if ( shadow_map_ )
	{
		cascade_count = std::min( shadow_map_->get_cascade_levels(), ecs::ObjectRenderDataSystem::MaxShadowCascades );

		for ( int n = 0; n < cascade_count; n++ )
		{
			Matrix view_projection;

			if ( ! shadow_map_->get_cascade_view_projection( n, view_projection ) )
			{
				cascade_count = 0;
				break;
			}

			shadow_frustums[ n ] = core::graphics::Frustum::from_view_projection( view_projection );
		}
	}

	// 影の段の範囲が無いときは、影の段を全部描く（何も外さない範囲を渡す）
	if ( shadow_map_ && cascade_count == 0 )
	{
		cascade_count = std::min( shadow_map_->get_cascade_levels(), ecs::ObjectRenderDataSystem::MaxShadowCascades );

		for ( int n = 0; n < cascade_count; n++ )
		{
			shadow_frustums[ n ] = core::graphics::Frustum();
		}
	}

	system->set_culling( core::graphics::Frustum::from_view_projection( camera_view_projection ), shadow_frustums, cascade_count );
}

/**
 * シャドウマップを描画する
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

		const auto* system = get_object_render_data_system();

		if ( ! system )
		{
			continue;
		}

		system->resolve_models();	// 1 フレームに 1 回だけ引く（段ごと・カスケードごとに引き直さない）

		// スキンメッシュでない物は、同じモデルの物をまとめて描く（準備ができていなければ、下で今までどおり 1 個ずつ描く）。
		// スキンメッシュは今までどおり、下で 1 個ずつ描く
		if ( ! is_skin_mesh && n < ecs::ObjectRenderDataSystem::MaxShadowCascades )
		{
			if ( const auto* instances = get_ready_object_instance_buffer( system ) )
			{
				render_shadow_map_instances( shader, * instances, * system, n );
				continue;
			}

			// 1 個ずつ描くので、定数バッファに書くのを後回しにした物の定数を書く（スキンメッシュは後回しにしないので、スキンメッシュの段では要らない）
			system->write_deferred_constants();
		}

		const std::uint8_t cascade_bit = ecs::ObjectRenderDataSystem::visible_in_shadow_cascade( n );

		for ( const auto& target : system->get_render_targets() )
		{
			const Model* model = target.model;

			// このカスケードの範囲の外の物は描かない（set_object_culling を呼んだフレームだけ）
			if ( ! model || ! system->is_visible( target, cascade_bit ) )
			{
				continue;
			}

			if ( model->is_skin_mesh() == is_skin_mesh )
			{
				set_current_object_constant_buffer( target.render->object_constant_buffer.get() );
				set_current_skinning_constant_buffer( target.render->bone_constant_buffer.get() );

				render_object_mesh( * target.render, model, shader );

				if ( model->get_line() && model->get_line()->is_cast_shadow() )
				{
					render_object_line( * target.render, model );
				}
			}
		}
	}
}

/**
 * 影のカスケード level のシャドウマップに、スキンメッシュでない物を、同じモデルの物をまとめて描く
 *
 * メッシュ → 影を落とす線の順に描く（以前は物体ごとにメッシュ・線の順。シャドウマップは深さだけで、深さの比べ方は描く順によらない）。
 * 以前は線を、直前に当てたメッシュのシャドウマップのパスのまま描いていた。今は線用のパス（shadow_map_line_instanced。同じ計算）で描く
 */
void GraphicsManager::render_shadow_map_instances( const BaseShadowMapShader* shader, const core::graphics::ObjectInstanceBuffer& instances, const ecs::ObjectRenderDataSystem& system, int level ) const
{
	const auto& targets = system.get_render_targets();

	// メッシュ
	{
		const auto& list = system.get_draw_list( ecs::ObjectRenderDataSystem::shadow_mesh_pass( level ) );

		object_instance_buffer_->update_indices( list.instance_indices.data(), list.instance_indices.size() );

		for ( const auto& item : list.items )
		{
			if ( item.batch == ecs::ObjectRenderDataSystem::NoBatch )
			{
				// 影の段はスキンメッシュでない物をすべてまとめるので、ここへは来ない（来たら今までどおり 1 個ずつ描く）
				const auto& target = targets[ item.target ];

				set_current_object_constant_buffer( target.render->object_constant_buffer.get() );
				set_current_skinning_constant_buffer( target.render->bone_constant_buffer.get() );

				render_object_mesh( * target.render, target.model, shader );
				continue;
			}

			const auto& batch = list.batches[ item.batch ];
			const Mesh* mesh = batch.model->get_mesh();

			instances.bind_to_vs( batch.first );

			mesh->bind();

			for ( uint_t m = 0; m < mesh->get_rendering_vertex_group_count(); m++ )
			{
				shader->render_object_instances( mesh, m, batch.count );
			}
		}
	}

	// 影を落とす線
	{
		const auto& list = system.get_draw_list( ecs::ObjectRenderDataSystem::shadow_line_pass( level ) );

		object_instance_buffer_->update_indices( list.instance_indices.data(), list.instance_indices.size() );

		for ( const auto& item : list.items )
		{
			if ( item.batch == ecs::ObjectRenderDataSystem::NoBatch )
			{
				continue;	// 線はすべてまとめるので、ここへは来ない
			}

			const auto& batch = list.batches[ item.batch ];

			instances.bind_to_vs( batch.first );
			shader->render_line_instances( batch.model->get_line(), batch.count );
		}
	}
}

/**
 * 全てのゲームの物体を描画する（描く対象は ecs::ObjectRenderDataSystem の一覧。以前は ActiveObjectManager の一覧）
 *
 */
void GraphicsManager::render_objects() const
{
	render_object_meshes();
	render_object_lines();
}

/// 物体のメッシュを描く（render_objects の前半）
void GraphicsManager::render_object_meshes() const
{
	if ( shadow_map_ )
	{
		shadow_map_->ready_to_render_scene();
	}

	const auto* system = get_object_render_data_system();

	if ( ! system )
	{
		return;
	}

	system->resolve_models();	// 1 フレームに 1 回だけ引く

	// 同じモデルの物はまとめて描く（準備ができていなければ、下で今までどおり 1 個ずつ描く）
	if ( const auto* instances = get_ready_object_instance_buffer( system ) )
	{
		const auto& list = system->get_draw_list( ecs::ObjectRenderDataSystem::MeshPass );
		const auto& targets = system->get_render_targets();

		object_instance_buffer_->update_indices( list.instance_indices.data(), list.instance_indices.size() );

		for ( const auto& item : list.items )
		{
			if ( item.batch == ecs::ObjectRenderDataSystem::NoBatch )
			{
				const auto& target = targets[ item.target ];

				set_current_object_constant_buffer( target.render->object_constant_buffer.get() );
				set_current_skinning_constant_buffer( target.render->bone_constant_buffer.get() );

				render_object_mesh( * target.render, target.model );
			}
			else
			{
				const auto& batch = list.batches[ item.batch ];

				instances->bind_to_vs( batch.first );
				batch.model->render_object_instances( batch.count );
			}
		}

		return;
	}

	// 1 個ずつ描くので、定数バッファに書くのを後回しにした物の定数を書く
	system->write_deferred_constants();

	for ( const auto& target : system->get_render_targets() )
	{
		const Model* model = target.model;

		// カメラの視錐台の外の物は描かない（set_object_culling を呼んだフレームだけ）
		if ( ! model || ! system->is_visible( target, ecs::ObjectRenderDataSystem::VisibleInCamera ) )
		{
			continue;
		}

		set_current_object_constant_buffer( target.render->object_constant_buffer.get() );
		set_current_skinning_constant_buffer( target.render->bone_constant_buffer.get() );

		render_object_mesh( * target.render, model );
	}
}

/// 物体の線を描く（render_objects の後半）
void GraphicsManager::render_object_lines() const
{
	const auto* system = get_object_render_data_system();

	set_input_layout( "pos_color" );

	// 線はどれも、同じモデルの物をまとめて描く（準備ができていなければ、下で今までどおり 1 個ずつ描く）
	if ( system )
	{
		system->resolve_models();	// 1 フレームに 1 回だけ引く（引いた後なら何もしない）

		if ( const auto* instances = get_ready_object_instance_buffer( system ) )
		{
			const auto& list = system->get_draw_list( ecs::ObjectRenderDataSystem::LinePass );

			object_instance_buffer_->update_indices( list.instance_indices.data(), list.instance_indices.size() );

			render_technique( "|drawing_line_instanced", [ this, & list, instances ]
			{
				get_game_render_data()->bind_to_all();
				get_frame_render_data()->bind_to_all();
				get_frame_drawing_render_data()->bind_to_all();

				for ( const auto& item : list.items )
				{
					if ( item.batch == ecs::ObjectRenderDataSystem::NoBatch )
					{
						continue;	// 線はすべてまとめるので、ここへは来ない
					}

					const auto& batch = list.batches[ item.batch ];

					instances->bind_to_vs( batch.first );
					batch.model->get_line()->render_instanced( batch.count );
				}
			} );

			return;
		}

		// 1 個ずつ描くので、定数バッファに書くのを後回しにした物の定数を書く
		system->write_deferred_constants();
	}

	render_technique( "|drawing_line", [ this, system ]
	{
		get_game_render_data()->bind_to_all();
		get_frame_render_data()->bind_to_all();
		get_frame_drawing_render_data()->bind_to_all();

		if ( ! system )
		{
			return;
		}

		system->resolve_models();	// 1 フレームに 1 回だけ引く（引いた後なら何もしない）

		for ( const auto& target : system->get_render_targets() )
		{
			const Model* model = target.model;

			if ( model && system->is_visible( target, ecs::ObjectRenderDataSystem::VisibleInCamera ) )
			{
				render_object_line( * target.render, model );
			}
		}
	} );
}

/**
 * 全 ActiveObject をモーションベクトル ( 速度 ) パスで描画する ( 案B / 段階0b )
 *
 * 呼び出し側で速度用レンダーターゲットをバインド・クリアしておくこと。
 * シーン深度を再利用するため、メインパス ( render_active_objects ) の直後に呼ぶこと。
 */
void GraphicsManager::render_objects_velocity() const
{
	auto* velocity_shader = const_cast< GraphicsManager* >( this )->get_shader< shader::VelocityShader >( "velocity" );

	if ( ! velocity_shader )
	{
		return;
	}

	get_frame_render_data()->bind_to_all();

	const auto* system = get_object_render_data_system();

	if ( ! system )
	{
		return;
	}

	system->resolve_models();	// 1 フレームに 1 回だけ引く
	system->write_deferred_constants();	// 物体の定数バッファを読むので、書くのを後回しにした物の定数を書く

	for ( const auto& target : system->get_render_targets() )
	{
		const Model* model = target.model;

		// カメラの視錐台の外の物は描かない（定数を書いていないことがある）
		if ( ! model || ! system->is_visible( target, ecs::ObjectRenderDataSystem::VisibleInCamera ) )
		{
			continue;
		}

		set_current_object_constant_buffer( target.render->object_constant_buffer.get() );
		set_current_skinning_constant_buffer( target.render->bone_constant_buffer.get() );

		velocity_shader->render_model( model );
	}
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

core::graphics::Shader* GraphicsManager::get_post_effect_shader() const
{
	return post_effect_rectangle_->get_shader_at( 0 );
}

/**
 * ポストエフェクトを BackBuffer にレンダリングする
 *
 * @param t 使用するテクスチャ
 */
void GraphicsManager::render_post_effect( Texture* texture )
{
	render_post_effect( texture, get_back_buffer_texture() );
}

/**
 * ポストエフェクトを指定した RenderTarget にレンダリングする
 *
 * @param texture 使用するテクスチャ
 * @param render_target レンダリング対象の RenderTarget
 */
void GraphicsManager::render_post_effect( Texture* texture, RenderTarget* render_target )
{
	// 描画結果を読む。MSAA の描画先は Texture2D として読めないので、MSAA なしへ解決してから使う
	// （以前は解決しておらず、ECS のシーン（physics_test など）の画面が真っ黒になっていた）
	Texture* source = texture;

	if ( auto* render_target_texture = dynamic_cast< core::graphics::RenderTargetTexture* >( texture ) )
	{
		source = render_target_texture->resolve_for_sampling();
	}

	set_render_target( render_target );

	post_effect_rectangle_->get_shader_at( 0 )->set_texture_at( 0, source );
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
	buffer_data.world = Matrix().set_identity();
	buffer_data.color = direct_3d_->getFader()->get_color();
	
	get_shared_object_render_data()->update( & buffer_data );

	set_input_layout( "2d" );

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
void GraphicsManager::render_debug_axis() const
{
	if ( ! is_debug_axis_enabled_ )
	{
		return;
	}

	const auto* system = get_object_render_data_system();

	if ( ! system )
	{
		return;
	}

	set_input_layout( "pos_color" );

	system->resolve_models();	// 1 フレームに 1 回だけ引く
	system->write_deferred_constants();	// 物体の定数バッファを読むので、書くのを後回しにした物の定数を書く

	render_technique( "|simple_line", [ this, system ]
	{
		get_frame_render_data()->bind_to_all();

		for ( const auto& target : system->get_render_targets() )
		{
			// カメラの視錐台の外の物は、定数を書いていないことがあるので描かない
			if ( ! target.render->object_constant_buffer || ! system->is_visible( target, ecs::ObjectRenderDataSystem::VisibleInCamera ) )
			{
				continue;
			}

			bind_object_render_data( * target.render );

			render_debug_axis_model();

			// ボーンの軸は描いている物体だけ（以前は死んでいない物体。死ぬと描かなくなるので、ほぼ同じ）
			const auto* animation = target.animation;

			if ( animation && animation->player && ( target.render->should_draw_mesh() || target.render->should_draw_line() ) )
			{
				render_debug_axis_for_bones( target.transform->transform, * animation->player, target.model );
			}
		}
	} );
}

/**
 * 物体の全ボーンに対してデバッグ用の軸を表示する
 *
 */
void GraphicsManager::render_debug_axis_for_bones( const core::Transform& t, const core::AnimationPlayer& animation_player, const Model* model ) const
{
	get_shared_object_render_data()->bind_to_vs();
	get_shared_object_render_data()->bind_to_ps();

	core::BoneConstantBufferData bone_constant_buffer_data;
	animation_player.calculate_bone_matrix_recursive( bone_constant_buffer_data, 0, Matrix::identity() );

	for ( uint_t n = 0; n < animation_player.get_skinning_animation_set()->get_bone_count(); ++n )
	{
		const Matrix& bone_offset_matrix = animation_player.get_skinning_animation_set()->get_bone_offset_matrix_by_bone_index( n );
		const Matrix& bone_matrix = bone_offset_matrix * bone_constant_buffer_data.bone_matrix[ n ];
		// const Matrix& bone_matrix = bone_offset_matrix;

		ObjectConstantBufferData shader_data;

		shader_data.world = Matrix().set_scaling( 0.1f, 0.1f, 0.1f );
		shader_data.world *= bone_matrix;
		shader_data.world *= Matrix().set_rotation_quaternion( t.get_rotation() );
		shader_data.world *= Matrix().set_translation( t.get_position().x(), t.get_position().y(), t.get_position().z() );

		if ( model && model->get_line() )
		{
			shader_data.color = model->get_line()->get_color();
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
	std::cout << "shader    : "; get_shader_manager().debug_print_resources();
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

	frame_constant_buffer_data.view = ( Matrix().set_look_at( eye, at, up ) );
	frame_constant_buffer_data.projection = Matrix().set_perspective_fov( math::degree_to_radian( camera_->fov() ), camera_->aspect(), camera_->near_clip(), camera_->far_clip() );

	get_frame_render_data()->update( & frame_constant_buffer_data );
}
*/

} // namespace blue_sky::graphics
