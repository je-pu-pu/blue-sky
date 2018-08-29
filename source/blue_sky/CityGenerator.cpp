#include "CityGenerator.h"
#include "graphics/Mesh.h"
#include "graphics/GraphicsManager.h"
#include "graphics/shader/FlatShader.h"
#include <GameMain.h>

#include <blue_sky/type.h>

#include <common/random.h>

namespace blue_sky
{

CityGenerator::CityGenerator()
	: model_( GameMain::get_instance()->get_graphics_manager()->create_named_model( "generated_city" ) )
	, control_point_( 0.f, 0.f, 0.f )
{
	model_->set_mesh( GameMain::get_instance()->get_graphics_manager()->create_named_mesh( "generated_city", Mesh::Buffer::Type::UPDATABLE ) );
	
	model_->set_shader_at( 0, GameMain::get_instance()->get_graphics_manager()->create_shader< graphics::shader::FlatShader >() );
	model_->get_shader_at( 0 )->set_texture_at( 0, GameMain::get_instance()->get_graphics_manager()->load_texture( "media/model/road.png" ) );

	model_->set_shader_at( 1, GameMain::get_instance()->get_graphics_manager()->create_shader< graphics::shader::FlatShader >() );
	model_->get_shader_at( 1 )->set_texture_at( 0, GameMain::get_instance()->get_graphics_manager()->load_texture( "media/model/road-cross.png" ) );

	model_->get_mesh()->create_vertex_group();
	model_->get_mesh()->create_vertex_group();

	model_->get_mesh()->create_vertex_buffer();
	model_->get_mesh()->create_index_buffer();

	step();
}

/**
 * ŠX‚ð¶¬‚·‚é
 *
 */
void CityGenerator::step()
{
	// extend_road();

	/**
	 *    ....... road_control_point2
	 * 2--.--3
	 * |     |
	 * |     |
	 * 0--.--1
	 *    ....... road_control_point1
	 */

	// extend_road( control_point_, control_point_ - Vector( 4.f, 0.f, 0.f ), control_point_ + Vector( 4.f, 0.f, 0.f ), 0, 40, 5 );

	road_node_list_.emplace_back( Vector( 0.f, 0.f, 0.f ) );
	road_node_list_.emplace_back( Vector( 0.f, 0.f, 1.f ) );
	road_node_list_.emplace_back( Vector( 0.f, 0.f, 2.f ) );
	road_node_list_.emplace_back( Vector( 0.f, 0.f, 3.f ) );

	for ( auto& node : road_node_list_ )
	{
		node.position *= 8.f;
	}

	road_node_list_[ 0 ].link_front( road_node_list_[ 1 ] );
	road_node_list_[ 1 ].link_front( road_node_list_[ 2 ] );
	road_node_list_[ 2 ].link_front( road_node_list_[ 3 ] );

	generate_road_mesh();

	model_->get_mesh()->create_vertex_buffer();
	model_->get_mesh()->create_index_buffer();
}

/**
 * “¹˜H‚ðL‚Î‚·
 *
 */
void CityGenerator::extend_road( const Vector& control_point, const Vector& vertex_position0, const Vector& vertex_position1, float_t direction, int life, int straight_life )
{
	const float_t road_depth = 8.f;
	const float_t road_width = 8.f;

	Matrix m1;
	m1.set_rotation_y( math::degree_to_radian( direction ) );
	const Vector right1 = Vector::Right * m1;

	Matrix m2;
	m2.set_rotation_y( math::degree_to_radian( direction ) );
	
	const Vector control_point2 = control_point + Vector::Forward * m2 * road_depth;
	const Vector right2 = Vector::Right * m2;

	const auto index_offset = model_->get_mesh()->get_vertex_count();

	const Vector vertex_position2 = Vector( control_point2 - right2 * road_width * 0.5f );
	const Vector vertex_position3 = Vector( control_point2 + right2 * road_width * 0.5f );

	model_->get_mesh()->add_vertex( Mesh::Vertex( { vertex_position0.xyz() }, { 0.f, 1.f, 0.f }, { 0.f, 1.f } ) ); // 0
	model_->get_mesh()->add_vertex( Mesh::Vertex( { vertex_position1.xyz() }, { 0.f, 1.f, 0.f }, { 0.f, 0.f } ) ); // 1

	model_->get_mesh()->add_vertex( Mesh::Vertex( { vertex_position2.xyz() }, { 0.f, 1.f, 0.f }, { 1.f, 1.f } ) ); // 2
	model_->get_mesh()->add_vertex( Mesh::Vertex( { vertex_position3.xyz() }, { 0.f, 1.f, 0.f }, { 1.f, 0.f } ) ); // 3

	const bool is_cross = straight_life <= 0 && common::random( 0, 2 ) == 0;
	auto* vertex_group = get_model()->get_mesh()->get_vertex_group_at( is_cross ? 1 : 0 );

	vertex_group->add_index( index_offset + 0 );
	vertex_group->add_index( index_offset + 2 );
	vertex_group->add_index( index_offset + 1 );

	vertex_group->add_index( index_offset + 1 );
	vertex_group->add_index( index_offset + 2 );
	vertex_group->add_index( index_offset + 3 );

	//
	// control_point_ = control_point2;
	
	if ( life )
	{
		if ( is_cross )
		{
			straight_life = 5;

			extend_road( ( vertex_position0 + vertex_position2 ) / 2.f, vertex_position0, vertex_position2, direction - 90.f, life / 2, straight_life );
			extend_road( ( vertex_position3 + vertex_position1 ) / 2.f, vertex_position3, vertex_position1, direction + 90.f, life / 2, straight_life );
		}

		extend_road( control_point2, vertex_position2, vertex_position3, direction + common::random( 0.f, +10.f ), life - 1, straight_life - 1 );
	}
}

/**
* ƒƒbƒVƒ…‚ð¶¬‚·‚é
*
*/
void CityGenerator::generate_road_mesh()
{
	for ( auto& node : road_node_list_ )
	{
		generate_road_mesh( node );
	}
}

/**
 * ƒƒbƒVƒ…‚ð¶¬‚·‚é
 *
 */
void CityGenerator::generate_road_mesh( const RoadNode& node )
{
	const float_t road_depth = 8.f;
	const float_t road_width = 8.f;

	const RoadNode* prev_node = node.back_node;
	const RoadNode* next_node = node.front_node;

	Vector start_front;
	Vector start_right;

	Vector end_front;
	Vector end_right;

	if ( prev_node )
	{
		start_front = ( node.position - prev_node->position ).normalize();
		start_right = Vector::Up.cross( start_front );
	}

	if ( next_node )
	{
		end_front = ( next_node->position - node.position ).normalize();
		end_right = Vector::Up.cross( end_front );
	}

	if ( ! prev_node )
	{
		start_front = end_front;
		start_right = end_right;
	}

	if ( ! next_node )
	{
		end_front = start_front;
		end_right = start_right;
	}

	auto road_front = start_front;
	
	auto road_start_pos = node.position - road_front * road_depth * 0.5f;
	auto road_end_pos   = node.position + road_front * road_depth * 0.5f;

	auto vertex_position0 = road_start_pos - start_right * road_width * 0.5f;
	auto vertex_position1 = road_start_pos + start_right * road_width * 0.5f;
	auto vertex_position2 = road_end_pos - end_right * road_width * 0.5f;
	auto vertex_position3 = road_end_pos + end_right * road_width * 0.5f;
	
	const auto index_offset = model_->get_mesh()->get_vertex_count();

	model_->get_mesh()->add_vertex( Mesh::Vertex( { vertex_position0.xyz() }, { 0.f, 1.f, 0.f }, { 0.f, 1.f } ) ); // 0
	model_->get_mesh()->add_vertex( Mesh::Vertex( { vertex_position1.xyz() }, { 0.f, 1.f, 0.f }, { 0.f, 0.f } ) ); // 1

	model_->get_mesh()->add_vertex( Mesh::Vertex( { vertex_position2.xyz() }, { 0.f, 1.f, 0.f }, { 1.f, 1.f } ) ); // 2
	model_->get_mesh()->add_vertex( Mesh::Vertex( { vertex_position3.xyz() }, { 0.f, 1.f, 0.f }, { 1.f, 0.f } ) ); // 3

	const bool is_cross = node.left_node && node.right_node;
	auto* vertex_group = get_model()->get_mesh()->get_vertex_group_at( is_cross ? 1 : 0 );

	vertex_group->add_index( index_offset + 0 );
	vertex_group->add_index( index_offset + 2 );
	vertex_group->add_index( index_offset + 1 );

	vertex_group->add_index( index_offset + 1 );
	vertex_group->add_index( index_offset + 2 );
	vertex_group->add_index( index_offset + 3 );

	/*
	if ( ! next_node.link_list.empty() )
	{
		generate_road_mesh( next_node, ** next_node.link_list.cbegin() );
	}
	*/
}

} // namespace blue_sky
