#include "GeometryShaderCanvasTestScene.h"

#include <blue_sky/graphics/GraphicsManager.h>

#include <core/ecs/component/TransformComponent.h>
#include <core/ecs/component/TransformControlComponent.h>
#include <core/ecs/component/CameraComponent.h>
#include <core/ecs/component/ModelComponent.h>
#include <core/ecs/component/RenderComponent.h>

#include <core/ecs/system/RenderSystem.h>
#include <core/ecs/system/TransformControlSystem.h>
#include <core/ecs/EntityManager.h>

#include <core/graphics/Mesh.h>

#include <blue_sky/Input.h>
#include <game/MainLoop.h>

#include <common/math.h>

#include <imgui.h>

#include <sstream>
#include <filesystem>
#include <regex>

namespace blue_sky
{

GeometryShaderCanvasTestScene::GeometryShaderCanvasTestScene()
	: mesh_( get_graphics_manager()->create_named_mesh( "line_cube", Mesh::Buffer::Type::DEFAULT ) )
	// , input_layout_( get_graphics_manager()->create_named_input_layout( "line_cube", { } ) )
	, input_layout_( get_graphics_manager()->get_input_layout( "line_cube" ) )

	, camera_( EntityManager::get_instance()->create_entity() )

	, texture_( get_graphics_manager()->load_texture( "media/texture/pen/white-hard-pen.png" ) )
{

	// 色
	const Color& c1 = Color::Red;
	const Color& c2 = Color::Green;
	const Color& c3 = Color::Blue;

	// 頂点
	mesh_->add_vertex( Vertex( Vector3( -1.f,  1.f, -1.f ), c1 ) );
	mesh_->add_vertex( Vertex( Vector3(  1.f,  1.f, -1.f ), c2 ) );
	mesh_->add_vertex( Vertex( Vector3( -1.f, -1.f, -1.f ), c3 ) );
	mesh_->add_vertex( Vertex( Vector3(  1.f, -1.f, -1.f ), c1 ) );
	mesh_->add_vertex( Vertex( Vector3( -1.f,  1.f,  1.f ), c2 ) );
	mesh_->add_vertex( Vertex( Vector3(  1.f,  1.f,  1.f ), c3 ) );
	mesh_->add_vertex( Vertex( Vector3( -1.f, -1.f,  1.f ), c1 ) );
	mesh_->add_vertex( Vertex( Vector3(  1.f, -1.f,  1.f ), c2 ) );
	
	// インデックス
	auto vertex_group = mesh_->get_vertex_group_at( 0 );
	vertex_group->set_index( {
		0, 1,
		0, 2,
		1, 3,
		2, 3,

		4, 5,
		4, 6,
		5, 7,
		6, 7,

		0, 4,
		1, 5,
		2, 6,
		3, 7,
	} );

	mesh_->create_vertex_buffer();
	mesh_->create_index_buffer();
	mesh_->clear_vertex_list();


	// get_graphics_manager()->create_input_layout( )

	// auto vertex_buffer = get_graphics_manager()->create_vertex_buffer( vs );
	// get_graphics_manager()->set_vertex_buffer( vertex_buffer );

	get_entity_manager()->add_system< core::ecs::TransformControlSystem >();

	camera_transform_ = camera_->add_component< core::ecs::TransformComponent >();
	camera_transform_->transform.set_identity();
	camera_transform_->transform.set_position( Vector( 0.f, 0.f, -10.f ) );

	camera_->add_component< core::ecs::TransformControlComponent >();
	camera_component_ = camera_->add_component< core::ecs::CameraComponent >();

	get_graphics_manager()->set_main_camera_info( camera_transform_, camera_component_ );
}

GeometryShaderCanvasTestScene::~GeometryShaderCanvasTestScene()
{
	get_entity_manager()->clear();
}

GeometryShaderCanvasTestScene::EntityManager* GeometryShaderCanvasTestScene::get_entity_manager()
{
	return EntityManager::get_instance();
}

void GeometryShaderCanvasTestScene::update()
{
	Scene::update();

	get_entity_manager()->update();

	if ( get_input()->push( Input::Button::B ) )
	{
		const std::array< const char_t*, 7 > texture_names = {
			"media/texture/pen/white-soft-pen.png",
			"media/texture/pen/white-hard-pen.png",
			"media/texture/pen/bump-hard-pen.png",
			"media/texture/pen/bump-cross-pen.png",
			"media/texture/pen/white-grass-pen.png",
			"media/texture/pen/white-hard-round-rect-pen.png",
			"media/texture/pen/white-hard-round-rect-stroke-pen.png"
		};

		static int i = 0;
		i = ( i + 1 ) % texture_names.size();

		texture_ = get_graphics_manager()->load_texture( texture_names[ i ] );
	}
}

void GeometryShaderCanvasTestScene::render()
{
	get_graphics_manager()->set_default_render_target();

	get_graphics_manager()->setup_rendering();
	get_graphics_manager()->render_background();

	get_graphics_manager()->set_primitive_topology( GraphicsManager::PrimitiveTopology::LINE_LIST );
	get_graphics_manager()->set_input_layout( input_layout_ );

	blue_sky::ObjectConstantBufferWithData shader_data;

	static const int bpm = 120;
	static int last_step = 0;
	const int step = static_cast< int >( std::round( get_total_elapsed_time() * ( bpm / 60 ) * 8 ) );

	static float a = 0.f;
	static float aa = 0.f;
	float scale = 1.f;

	a += 0.01f;

	if ( step != last_step )
	{
		aa = a;

		last_step = step;
	}

	if ( step / 2 % 4 == 0 )
	{
		scale = 1.05f;
	}
	

	shader_data.data().world.set_identity();
	shader_data.data().world *= Matrix().set_rotation_y( aa );
	shader_data.data().world *= Matrix().set_scaling( scale, scale, scale );
	shader_data.data().world = shader_data.data().world.transpose();

	shader_data.update();

	get_graphics_manager()->set_current_object_constant_buffer( & shader_data );
	get_graphics_manager()->set_current_skinning_constant_buffer( nullptr );

	render_technique( "|line_cube", [ this ]
	{
		get_graphics_manager()->get_game_render_data()->bind_to_all();
		get_graphics_manager()->get_frame_render_data()->bind_to_all();
		get_graphics_manager()->get_frame_drawing_render_data()->bind_to_all();
		get_graphics_manager()->get_current_object_constant_buffer()->bind_to_all();

		texture_->bind_to_ps( 1 );

		mesh_->bind();
		mesh_->render( 0 );
	} );
}

} // namespace blue_sky
