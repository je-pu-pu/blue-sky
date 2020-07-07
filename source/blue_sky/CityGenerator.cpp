#include "CityGenerator.h"
#include "graphics/Mesh.h"
#include "graphics/GraphicsManager.h"
#include "graphics/shader/FlatShader.h"
#include <GameMain.h>

#include <blue_sky/type.h>
#include <core/math/LineSegment2.h>

#include <common/random.h>

#include <iostream>

namespace blue_sky
{

CityGenerator::CityGenerator()
	: model_( GameMain::get_instance()->get_graphics_manager()->create_named_model( "generated_city" ) )
	, debug_model_( GameMain::get_instance()->get_graphics_manager()->create_named_model( "generated_city_debug" ) )
{
	model_->set_mesh( GameMain::get_instance()->get_graphics_manager()->create_named_mesh( "generated_city", Mesh::Buffer::Type::UPDATABLE ) );
	
	model_->set_shader_at( 0, GameMain::get_instance()->get_graphics_manager()->create_shader< graphics::shader::FlatShader >() );
	model_->get_shader_at( 0 )->set_texture_at( 0, GameMain::get_instance()->get_graphics_manager()->load_texture( "media/model/road.png" ) );

	model_->set_shader_at( 1, GameMain::get_instance()->get_graphics_manager()->create_shader< graphics::shader::FlatShader >() );
	model_->get_shader_at( 1 )->set_texture_at( 0, GameMain::get_instance()->get_graphics_manager()->load_texture( "media/model/road-cross.png" ) );

	model_->get_mesh()->create_vertex_group();
	model_->get_mesh()->create_vertex_group();

	road_node_list_.reserve( 1024 * 4 );
	road_node_list_.emplace_back( RoadNode::Type::STRAIGHT, Vector::Zero );

	road_control_point_list_.emplace_back( RoadControlPoint::Type::FRONT, Vector::Zero, Vector::Forward, & road_node_list_.back() );

	road_node_list_.emplace_back( RoadNode::Type::STRAIGHT, Vector( 100.f, 0.f, 0.f ) );
	road_control_point_list_.emplace_back( RoadControlPoint::Type::FRONT, Vector( 100.f, 0.f, 0.f ), Vector::Left, & road_node_list_.back() );

	road_node_list_.emplace_back( RoadNode::Type::STRAIGHT, Vector( 50.f, 0.f, 100.f ) );
	road_control_point_list_.emplace_back( RoadControlPoint::Type::FRONT, Vector( 50.f, 0.f, 100.f ), Vector::Back, & road_node_list_.back() );

	model_->get_mesh()->create_vertex_buffer();
	model_->get_mesh()->create_index_buffer();
}

/**
 * 街を生成するステップを進める
 *
 */
void CityGenerator::step()
{
	// extend_road( control_point_, control_point_ - Vector( 4.f, 0.f, 0.f ), control_point_ + Vector( 4.f, 0.f, 0.f ), 0, 40, 5 );
	
	// 新しく発生したコントロールポイント
	RoadControlPointList new_cps;

	for ( auto& cp : road_control_point_list_ )
	{
		cp.position += cp.front * get_road_depth();

		auto type = RoadNode::Type::STRAIGHT;

		const auto road_count = std::count_if( road_node_list_.begin(), road_node_list_.end(), [=] ( const auto& n ) { return ( n.position - cp.position ).length() <= get_road_width() * get_required_straight_road_count(); } );
		const auto is_corss_near = std::any_of( road_node_list_.begin(), road_node_list_.end(), [=] ( const auto& n ) { return n.type == RoadNode::Type::CROSS && ( n.position - cp.position ).length() <= get_road_width() * get_required_straight_road_count(); } );

		// if ( common::random( 0, 5 ) == 0 && cp.is_crossable() )
		// if ( road_count == get_required_straight_road_count() && ! is_corss_near )
		if ( false )
		{
			type = RoadNode::Type::CROSS;
		}

		road_node_list_.emplace_back( type, cp.position );
		road_node_list_.back().back_node = cp.node;

		if ( cp.type == RoadControlPoint::Type::FRONT )
		{
			cp.node->front_node = & road_node_list_.back();
		}
		else if ( cp.type == RoadControlPoint::Type::LEFT )
		{
			cp.node->left_node = & road_node_list_.back();
		}
		else if ( cp.type == RoadControlPoint::Type::RIGHT )
		{
			cp.node->right_node = & road_node_list_.back();
		}

		cp.node = & road_node_list_.back();
		cp.type = RoadControlPoint::Type::FRONT;

		if ( type != RoadNode::Type::CROSS )
		{
			Matrix m;
			m.set_rotation_y( math::degree_to_radian( common::random( -30.f, +30.f ) ) );
			// m.set_rotation_y( math::degree_to_radian( 20.f ) );
			cp.front *= m;
			cp.front.normalize();
		}

		if ( type == RoadNode::Type::CROSS )
		{
			new_cps.emplace_back( RoadControlPoint::Type::LEFT,  cp.position, cp.front.cross( Vector::Up ), & road_node_list_.back() );
			new_cps.emplace_back( RoadControlPoint::Type::RIGHT, cp.position, Vector::Up.cross( cp.front ), & road_node_list_.back() );
		}
	}

	road_control_point_list_.insert( road_control_point_list_.end(), new_cps.begin(), new_cps.end() );

	for ( auto& node : road_node_list_ )
	{
		node.update_vertex_pos();
	}

	std::vector< RoadNode* > del_nodes; // 削除するノード

	for ( const auto& cp : road_control_point_list_ )
	{
		const auto a = core::LineSegment2( cp.front_left_pos().xz(), cp.front_right_pos().xz() );

		for ( const auto& node : road_node_list_ )
		{
			if ( cp.node == & node )
			{
				continue;
			}

			const auto left_edge = core::LineSegment2( node.back_left_pos.xz(), node.front_left_pos.xz() );			///< 左の辺
			const auto front_edge = core::LineSegment2( node.front_left_pos.xz(), node.front_right_pos.xz() );		///< 前方の辺
			const auto right_edge = core::LineSegment2( node.front_right_pos.xz(), node.back_right_pos.xz() );		///< 右の辺

			auto hit_pos = a.intersection( left_edge );

			if ( ! hit_pos )
			{
				hit_pos = a.intersection( front_edge );
			}
			if ( ! hit_pos )
			{
				hit_pos = a.intersection( right_edge );
			}

			if ( hit_pos )
			{
				std::cout << "hit at : " << hit_pos.value() << ", cp = " << cp.position << std::endl;
				del_nodes.push_back( cp.node );
				break;
			}
		}
	}

	// 他の道路にぶつかったノードとコントロールポイントを削除する
	road_control_point_list_.erase( std::remove_if( road_control_point_list_.begin(), road_control_point_list_.end(),[&]( auto& cp ) { return std::find( del_nodes.begin(), del_nodes.end(), cp.node ) != del_nodes.end(); } ), road_control_point_list_.end() );
	road_node_list_.erase( std::remove_if( road_node_list_.begin(), road_node_list_.end(),[&]( auto& node ) { return std::find( del_nodes.begin(), del_nodes.end(), & node ) != del_nodes.end(); } ), road_node_list_.end() );

	std::cout << "--------------------" << std::endl;

	for ( auto& cp : road_control_point_list_ )
	{
		std::cout << "CP : " << static_cast< int >( cp.type ) << ", " << cp.position << std::endl;
	}

	// format_crossroad();

	std::cout << "cp count : " << road_control_point_list_.size() << std::endl;
	std::cout << "node cout : " << road_node_list_.size() << std::endl;

	generate_road_mesh();

	model_->get_mesh()->create_vertex_buffer();
	model_->get_mesh()->create_index_buffer();
}

/**
 * 交差した道路を整形する
 */
void CityGenerator::format_crossroad()
{
	auto a = core::LineSegment2::intersection( core::LineSegment2( Vector2( -1.f, 0.f ), Vector2( +1.f, 0.f ) ), core::LineSegment2( Vector2( 0.f, -1.f ), Vector2( 0.f, +1.f ) ) );
	auto b = core::LineSegment2::intersection( core::LineSegment2( Vector2( -1.f, -1.f ), Vector2( -1.f, +1.f ) ), core::LineSegment2( Vector2( -1.f, -1.f ), Vector2( +1.f, -1.f ) ) );

	if ( a )
	{
		std::cout << "a : " << a.value() << std::endl;
	}
	else
	{
		std::cout << "a : nullopt" << std::endl;
	}

	if ( b )
	{
		std::cout << "b : " << b.value() << std::endl;
	}
	else
	{
		std::cout << "b : nullopt" << std::endl;
	}
	

	return;

	for ( auto i = road_node_list_.begin(); i != road_node_list_.end(); ++i )
	{
		for ( auto j = i + 1; j != road_node_list_.end(); ++j )
		{
			if ( i->is_collition_with( *j ) )
			{
				std::cout << i->position << std::endl;
			}
		}
	}
}

#if 0
/**
 * 道路を伸ばす
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
#endif

/**
 * メッシュを生成する
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
 * 四隅の座標を計算し更新する
 */
void CityGenerator::RoadNode::update_vertex_pos()
{
	Vector start_front;
	Vector start_right;

	Vector end_front;
	Vector end_right;

	if ( back_node )
	{
		start_front = ( position - back_node->position ).normalize();
		start_right = Vector::Up.cross( start_front );
	}

	if ( front_node )
	{
		end_front = ( front_node->position - position ).normalize();
		end_right = Vector::Up.cross( end_front );
	}

	if ( ! back_node )
	{
		start_front = end_front;
		start_right = end_right;
	}

	if ( ! front_node )
	{
		end_front = start_front;
		end_right = start_right;
	}

	auto road_start_pos = position - start_front * get_road_depth() * 0.5f;
	auto road_end_pos   = position + end_front   * get_road_depth() * 0.5f;

	back_left_pos   = road_start_pos - start_right * get_road_width() * 0.5f;
	back_right_pos  = road_start_pos + start_right * get_road_width() * 0.5f;
	front_left_pos  = road_end_pos - end_right * get_road_width() * 0.5f;
	front_right_pos = road_end_pos + end_right * get_road_width() * 0.5f;
}

/**
 * メッシュを生成する
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

	const auto index_offset = model_->get_mesh()->get_vertex_count();

	model_->get_mesh()->add_vertex( Mesh::Vertex( { node.back_left_pos.xyz() }, { 0.f, 1.f, 0.f }, { 0.f, 1.f } ) ); // 0
	model_->get_mesh()->add_vertex( Mesh::Vertex( { node.back_right_pos.xyz() }, { 0.f, 1.f, 0.f }, { 0.f, 0.f } ) ); // 1

	model_->get_mesh()->add_vertex( Mesh::Vertex( { node.front_left_pos.xyz() }, { 0.f, 1.f, 0.f }, { 1.f, 1.f } ) ); // 2
	model_->get_mesh()->add_vertex( Mesh::Vertex( { node.front_right_pos.xyz() }, { 0.f, 1.f, 0.f }, { 1.f, 0.f } ) ); // 3

	const bool is_cross = node.type == RoadNode::Type::CROSS; // node.left_node && node.right_node;
	auto* vertex_group = get_model()->get_mesh()->get_vertex_group_at( is_cross ? 1 : 0 );

	vertex_group->add_index( index_offset + 0 );
	vertex_group->add_index( index_offset + 2 );
	vertex_group->add_index( index_offset + 1 );

	vertex_group->add_index( index_offset + 1 );
	vertex_group->add_index( index_offset + 2 );
	vertex_group->add_index( index_offset + 3 );

	/*
	if ( ! front_node.link_list.empty() )
	{
		generate_road_mesh( front_node, ** front_node.link_list.cbegin() );
	}
	*/
}

} // namespace blue_sky
