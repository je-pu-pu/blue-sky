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
{
	model_->set_mesh( GameMain::get_instance()->get_graphics_manager()->create_named_mesh( "generated_city", Mesh::Buffer::Type::UPDATABLE ) );
	
	model_->set_shader_at( 0, GameMain::get_instance()->get_graphics_manager()->create_shader< graphics::shader::FlatShader >() );
	model_->get_shader_at( 0 )->set_texture_at( 0, GameMain::get_instance()->get_graphics_manager()->load_texture( "media/model/road.png" ) );

	model_->set_shader_at( 1, GameMain::get_instance()->get_graphics_manager()->create_shader< graphics::shader::FlatShader >() );
	model_->get_shader_at( 1 )->set_texture_at( 0, GameMain::get_instance()->get_graphics_manager()->load_texture( "media/model/road-cross.png" ) );

	model_->get_mesh()->create_vertex_group();
	model_->get_mesh()->create_vertex_group();

	road_node_list_.reserve( 1024 * 4 );
	road_node_list_.emplace_back( Vector::Zero );

	road_control_point_list_.emplace_back( Vector::Zero, Vector::Forward, & road_node_list_.back() );

	model_->get_mesh()->create_vertex_buffer();
	model_->get_mesh()->create_index_buffer();
}

/**
 * ŠX‚ð¶¬‚·‚é
 *
 */
void CityGenerator::step()
{
	// extend_road( control_point_, control_point_ - Vector( 4.f, 0.f, 0.f ), control_point_ + Vector( 4.f, 0.f, 0.f ), 0, 40, 5 );

	for ( auto& cp : road_control_point_list_ )
	{
		cp.position += cp.front * get_road_depth();

		road_node_list_.emplace_back( cp.position );
		road_node_list_.back().back_node = cp.node;
		cp.node->front_node = & road_node_list_.back();

		cp.node = & road_node_list_.back();

		Matrix m;
		m.set_rotation_y( math::degree_to_radian( common::random( -5.f, +5.f ) ) );
		cp.front *= m;
		cp.front.normalize();
	}

	auto& cp = road_control_point_list_.back();
	const auto road_count = std::count_if( road_node_list_.begin(), road_node_list_.end(), [=] ( const auto& n ) { return ( n.position - cp.position ).length() <= get_road_width() * 3.f; } );

	if ( road_count <= 4 && common::random( 0, 5 ) == 0 )
	{
		road_control_point_list_.emplace_back( cp.position, Vector::Up.cross( cp.front ), & road_node_list_.back() );
		road_control_point_list_.emplace_back( cp.position, cp.front.cross( Vector::Up ), & road_node_list_.back() );
	}

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
	Matrix m1;
	m1.set_rotation_y( math::degree_to_radian( direction ) );
	const Vector right1 = Vector::Right * m1;

	Matrix m2;
	m2.set_rotation_y( math::degree_to_radian( direction ) );
	
	const Vector control_point2 = control_point + Vector::Forward * m2 * get_road_depth();
	const Vector right2 = Vector::Right * m2;

	const auto index_offset = model_->get_mesh()->get_vertex_count();

	const Vector vertex_position2 = Vector( control_point2 - right2 * get_road_width() * 0.5f );
	const Vector vertex_position3 = Vector( control_point2 + right2 * get_road_width() * 0.5f );

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
	model_->get_mesh()->clear_vertex_list();
	model_->get_mesh()->get_vertex_group_at( 0 )->clear();
	model_->get_mesh()->get_vertex_group_at( 1 )->clear();

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
	/**
	 * 2-------3
	 * |       |
	 * |   +   | 
	 * |       |
	 * 0-------1
	 *
	 * +      : node.position
	 * 0 .. 3 : vertex
	 */

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

	// auto road_front = end_front; // ( start_front + end_front ) * 0.5f;
	
	auto road_start_pos = node.position - start_front * get_road_depth() * 0.5f;
	auto road_end_pos   = node.position + end_front   * get_road_depth() * 0.5f;

	auto vertex_position0 = road_start_pos - start_right * get_road_width() * 0.5f;
	auto vertex_position1 = road_start_pos + start_right * get_road_width() * 0.5f;
	auto vertex_position2 = road_end_pos - end_right * get_road_width() * 0.5f;
	auto vertex_position3 = road_end_pos + end_right * get_road_width() * 0.5f;
	
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
