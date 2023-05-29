#include "TransformTestScene.h"

#include <blue_sky/graphics/GraphicsManager.h>

#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/component/TransformControlComponent.h>
#include <core/ecs/component/CameraComponent.h>
#include <core/ecs/component/ModelComponent.h>
#include <core/ecs/component/RenderComponent.h>

#include <core/ecs/system/RenderSystem.h>
#include <core/ecs/system/TransformControlSystem.h>
#include <core/ecs/EntityManager.h>

#include <game/MainLoop.h>

#include <common/math.h>

#include <imgui.h>

#include <sstream>
#include <filesystem>
#include <regex>

namespace blue_sky
{

TransformTestScene::TransformTestScene()
	: camera_( EntityManager::get_instance()->create_entity() )
{
	get_graphics_manager()->setup_default_shaders();
	get_graphics_manager()->load_paper_textures();

	// System を追加する
	get_entity_manager()->add_system< core::ecs::TransformControlSystem >();
	get_entity_manager()->add_system< core::ecs::RenderSystem >( 1000 );

	// Entity と Component を追加する
	current_entity_ = get_entity_manager()->create_entity();
	current_entity_transform_ = current_entity_->add_component< core::ecs::TransformComponent >();
	current_entity_transform_->transform.set_identity();
	current_entity_->add_component< core::ecs::RenderComponent >();
	current_entity_model_ = current_entity_->add_component< core::ecs::ModelComponent >();

	current_entity_model_->model = get_graphics_manager()->load_model( "goal" );

	camera_transform_ = camera_->add_component< core::ecs::TransformComponent >();
	camera_transform_->transform.set_identity();
	camera_transform_->transform.set_position( Vector( 0.f, 1.5f, -10.f ) );

	camera_->add_component< core::ecs::TransformControlComponent >();
	camera_component_ = camera_->add_component< core::ecs::CameraComponent >();

	get_graphics_manager()->set_main_camera_info( camera_transform_, camera_component_ );

	std::regex re(R"(.*\.bin\.fbx$)");

	for ( const auto & file : std::filesystem::directory_iterator( "media/model/" ) )
	{
		if ( ! std::regex_match( file.path().string(), re ) )
		{
			continue;
		}

		std::cout << file.path().stem().stem() << std::endl;
		model_file_name_list_.push_back( file.path().stem().stem().string() );
	}
}

TransformTestScene::~TransformTestScene()
{
	get_entity_manager()->clear();
}

TransformTestScene::EntityManager* TransformTestScene::get_entity_manager()
{
	return EntityManager::get_instance();
}

void TransformTestScene::update()
{
	Scene::update();

	get_graphics_manager()->update();
	get_graphics_manager()->clear_debug_bullet();

	get_entity_manager()->update();

	
	/// @todo Component の値を GUI で変更する仕組みを整理する
	
	// GUI によってモデルを変更する
	static int model_index = 0;
	std::vector< const char* > combo_items;

	std::for_each( model_file_name_list_.begin(), model_file_name_list_.end(), [&] ( const auto& s ) { combo_items.push_back( s.c_str() ); } );

	ImGui::Begin( "Model params" );
	if ( ImGui::Combo( "Model", & model_index, & combo_items[ 0 ], combo_items.size() ) )
	{
		std::cout << "index : " << model_index << std::endl;
		current_entity_model_->model = get_graphics_manager()->load_model( combo_items[ model_index ] );
	}

	static std::vector< int > shader_index_list;
	std::vector< const char* > shader_combo_items;

	// get_graphics_manager()->debug_print_resources();

	for ( const auto& s : get_graphics_manager()->get_shader_list() )
	{
		shader_combo_items.push_back( s->get_name() );
	}

	shader_index_list.resize( current_entity_model_->model->get_shader_count() );

	for ( int n = 0; n < current_entity_model_->model->get_shader_count(); n++ )
	{
		if ( ImGui::Combo( string_t( "Shader " + std::to_string( n ) ).c_str(), & shader_index_list[ n ], & shader_combo_items[ 0 ], shader_combo_items.size() ) )
		{
			std::cout << "index : " << shader_index_list[ n ] << std::endl;
			current_entity_model_->model->set_shader_at( n, get_graphics_manager()->get_shader_list()[ shader_index_list[ n ] ].get() );
		}
	}

	ImGui::End();

	// GUI によってオブジェクトの位置・回転を変更する
	static Vector rot;

	ImGui::Begin( "Transform test params" );
	ImGui::DragFloat3( "Position", reinterpret_cast< float* >( & current_entity_transform_->transform.get_position() ), 0.1f );
	ImGui::DragFloat3( "Rotation", reinterpret_cast< float* >( & rot ), 0.1f, -360.f, 360.f );
	ImGui::End();

	Quaternion q;
	Vector r = math::degree_to_radian( rot );
	q.set_euler_zyx( r.z(), r.y(), r.x() );
	current_entity_transform_->transform.set_rotation( q );

	// GUI によってカメラの情報を変更する
	ImGui::Begin( "Camera params" );
	ImGui::DragFloat( "FOV", & camera_component_->fov, 0.1f, 0.f, 180.f );
	ImGui::DragFloat( "Near clip", & camera_component_->near_clip, 0.1f, 0.1f, 3000.f );
	ImGui::DragFloat( "Far clip", & camera_component_->far_clip, 0.1f, 0.1f, 3000.f );
	ImGui::End();
}

void TransformTestScene::render()
{
	std::stringstream ss;
	ss << "Time : " << get_total_elapsed_time() << '\n';
	ss << "FPS : " << get_main_loop()->get_last_fps() << '\n';
	ss << "pass count : " << get_graphics_manager()->get_pass_count() << '\n';
	ss << "draw count : " << get_graphics_manager()->get_draw_count() << '\n';

	ss << "eye : "
		<< camera_transform_->transform.get_position().x() << ", "
		<< camera_transform_->transform.get_position().y() << ", "
		<< camera_transform_->transform.get_position().z() << '\n';
	// ss << "rot : " << camera_->rotate_degree().x() << ", " << camera_->rotate_degree().y() << ", " << camera_->rotate_degree().z() << '\n';

	ss << "psts "<< '\n';

	get_graphics_manager()->draw_text( 10.f, 10.f, static_cast< float >( get_width() ) - 10.f, static_cast< float >( get_height() ) - 10.f, ss.str().c_str(), Color::White );
}

} // namespace blue_sky
