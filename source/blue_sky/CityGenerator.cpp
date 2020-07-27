#include "CityGenerator.h"
#include "graphics/Mesh.h"
#include "graphics/GraphicsManager.h"
#include "graphics/shader/FlatShader.h"
#include <GameMain.h>

#include <blue_sky/type.h>
#include <core/math/LineSegment2.h>

#include <common/random.h>
#include <common/math.h>

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

	model_->set_shader_at( 2, GameMain::get_instance()->get_graphics_manager()->create_shader< graphics::shader::FlatShader >() );
	model_->get_shader_at( 2 )->set_texture_at( 0, GameMain::get_instance()->get_graphics_manager()->load_texture( "media/model/road-t.png" ) );

	for ( int n = 0; n < model_->get_shader_count(); n++ )
	{
		model_->get_mesh()->create_vertex_group();
	}

	road_node_list_.reserve( 1024 * 4 );
	road_node_list_.emplace_back( std::make_unique< RoadNode >( RoadNode::Type::STRAIGHT, Vector::Zero ) );

	road_control_point_list_.emplace_back( RoadControlPoint::Type::FRONT, Vector::Zero, Vector::Forward, road_node_list_.back().get() );

	road_node_list_.emplace_back( std::make_unique< RoadNode >( RoadNode::Type::STRAIGHT, Vector( 50.f, 0.f, 0.f ) ) );
	road_control_point_list_.emplace_back( RoadControlPoint::Type::FRONT, Vector( 50.f, 0.f, 0.f ), Vector::Left, road_node_list_.back().get() );

	road_node_list_.emplace_back( std::make_unique< RoadNode >( RoadNode::Type::STRAIGHT, Vector( 25.f, 0.f, 50.f ) ) );
	road_control_point_list_.emplace_back( RoadControlPoint::Type::FRONT, Vector( 25.f, 0.f, 50.f ), Vector::Back, road_node_list_.back().get() );

	model_->get_mesh()->create_vertex_buffer();
	model_->get_mesh()->create_index_buffer();

	debug_model_->set_mesh( GameMain::get_instance()->get_graphics_manager()->create_named_mesh( "generated_city_debug", Mesh::Buffer::Type::DEFAULT ) );
	
	debug_model_->set_shader_at( 0, GameMain::get_instance()->get_graphics_manager()->create_shader< graphics::shader::FlatShader >() );
	debug_model_->get_shader_at( 0 )->set_texture_at( 0, GameMain::get_instance()->get_graphics_manager()->load_texture( "media/model/road.png" ) );	

	debug_model_->get_mesh()->create_vertex_group();

	debug_model_->get_mesh()->create_vertex_buffer();
	debug_model_->get_mesh()->create_index_buffer();
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

	// 削除するコントロールポイント
	RoadControlPointList del_cps;

	std::vector< RoadNode* > del_nodes; // 削除するノード

	for ( auto& cp : road_control_point_list_ )
	{
		cp.position += cp.front * get_road_depth();

		if ( check_collision( cp ) )
		{
			del_cps.push_back( cp );
			continue;
		}

		auto type = RoadNode::Type::STRAIGHT;

		const auto road_count = std::count_if( road_node_list_.begin(), road_node_list_.end(), [=] ( const auto& n ) { return ( n->position - cp.position ).length() <= get_road_width() * get_required_straight_road_count(); } );
		const auto is_corss_near = std::any_of( road_node_list_.begin(), road_node_list_.end(), [=] ( const auto& n ) { return n->type == RoadNode::Type::CROSS && ( n->position - cp.position ).length() <= get_road_width() * get_required_straight_road_count(); } );

		// if ( common::random( 0, 5 ) == 0 && cp.is_crossable() )
		if ( road_count == get_required_straight_road_count() && ! is_corss_near )
		// if ( false )
		{
			type = RoadNode::Type::CROSS;
		}

		road_node_list_.emplace_back( std::make_unique< RoadNode >( type, cp.position ) );
		road_node_list_.back()->back_node = cp.node;

		if ( cp.type == RoadControlPoint::Type::FRONT )
		{
			cp.node->front_node = road_node_list_.back().get();
		}
		else if ( cp.type == RoadControlPoint::Type::LEFT )
		{
			cp.node->left_node = road_node_list_.back().get();
		}
		else if ( cp.type == RoadControlPoint::Type::RIGHT )
		{
			cp.node->right_node = road_node_list_.back().get();
		}

		cp.node = road_node_list_.back().get();
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
			new_cps.emplace_back( RoadControlPoint::Type::LEFT,  cp.position, cp.front.cross( Vector::Up ), road_node_list_.back().get() );
			new_cps.emplace_back( RoadControlPoint::Type::RIGHT, cp.position, Vector::Up.cross( cp.front ), road_node_list_.back().get() );
		}
	}

	road_control_point_list_.insert( road_control_point_list_.end(), new_cps.begin(), new_cps.end() );

	for ( auto& node : road_node_list_ )
	{
		node->update_vertex_pos();
	}

	// 他の道路にぶつかったノードとコントロールポイントを削除する
	road_control_point_list_.erase(
		std::remove_if(
			road_control_point_list_.begin(),
			road_control_point_list_.end(),
			[&]( auto& cp )
			{
				return std::find_if( del_cps.begin(), del_cps.end(), [&] ( auto& cp2 ) { return cp2.node == cp.node; } ) != del_cps.end();
			}
		),
		road_control_point_list_.end()
	);

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

	generate_debug_mesh();

	debug_model_->get_mesh()->create_vertex_buffer();
	debug_model_->get_mesh()->create_index_buffer();
}

/**
 * 伸ばしたコントロールポイントと既存の道ノードとの衝突判定を行う
 *
 * 衝突していた場合は伸ばした道路と既存の道路を接続する
 *
 * @param cp 伸ばしたコントロールポイント
 * @return コントロールポイントが既存の道ノードと衝突した場合は true を、衝突しなかった場合は false を返す
 */
bool CityGenerator::check_collision( RoadControlPoint& cp ) const
{
	for ( const auto& node : road_node_list_ )
	{
		if ( cp.node == node.get() )
		{
			continue;
		}

		if ( cp.node->back_node == node.get() )
		{
			continue;
		}

		// 当り判定の半径
		const auto radius = std::sqrt( get_road_width() * 2.f + get_road_depth() * 2.f ) * 1.5f;

		const auto lx = cp.position.x() - node->position.x();
		const auto lz = cp.position.z() - node->position.z();
		const auto length = std::sqrt( lx * lx + lz * lz );

		if ( length <= radius )
		{
			// 当たったノードと繋ぐ
			cp.node->front_node = node.get();
			cp.node->is_end = true;

			if ( node->type == RoadNode::Type::STRAIGHT )
			{
				node->type = RoadNode::Type::T_INTERSECTION;

				// @todo 共通化する
				const float cross = node->start_front.xz().cross( ( cp.position - node->position ).xz() );

				if ( cross < 0.f )
				{
					// 既存の道路に右からぶつかった場合
					node->right_node = cp.node;
				}
				else if ( cross > 0.f )
				{
					// 既存の道路に左からぶつかった場合
					node->left_node = cp.node;
				}
			}

			return true;
		}
	}

	return false;
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
			if ( ( *i )->is_collition_with( * j->get() ) )
			{
				std::cout << ( *i )->position << std::endl;
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

	for ( auto& vg : model_->get_mesh()->get_vertex_group_list() )
	{
		vg->clear();
	}

	for ( auto& node : road_node_list_ )
	{
		generate_road_mesh( node.get() );
	}
}

/**
 * 四隅の座標を計算し更新する
 */
void CityGenerator::RoadNode::update_vertex_pos()
{
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

	// 終端の処理
	if ( is_end )
	{
		const float cross = front_node->start_front.xz().cross( ( position - front_node->position ).xz() );

		if ( cross < 0.f )
		{
			// 既存の道路に右からぶつかった場合
			front_left_pos = front_node->back_right_pos;
			front_right_pos = front_node->front_right_pos;
		}
		else if ( cross > 0.f )
		{
			// 既存の道路に左からぶつかった場合
			front_left_pos = front_node->front_left_pos;
			front_right_pos = front_node->back_left_pos;
		}
		else if ( false )
		{
			// 既存の道路に正面からぶつかった場合
			front_left_pos = front_node->front_right_pos;
			front_right_pos = front_node->front_left_pos;
		}

		if ( ( position - front_node->position ).length() > get_road_width() * 3.f )
		{
			std::cout << "--- ??? ---" << std::endl;
			std::cout << "node : " << this << std::endl;
			std::cout << "front node : " << front_node << std::endl;
		}
	}
}

/**
 * メッシュを生成する
 *
 */
void CityGenerator::generate_road_mesh( const RoadNode* node )
{
	/**
	 * 分割数 0 の場合
	 *
	 * 2-------3
	 * |       |
	 * |   +   | 
	 * |       |
	 * 0-------1
	 *
	 *
	 * 分割数 1 の場合
	 *
	 * 4-------5
	 * |       |
	 * 2---+---3
	 * |       |
	 * 0-------1
	 *
	 * +      : node.position
	 * 0 .. 5 : vertex
	 */

	const auto index_offset = model_->get_mesh()->get_vertex_count();

	float rotation_degree = 0.f;

	if ( node->type == RoadNode::Type::STRAIGHT )
	{
		rotation_degree = 90.f;
	}
	else if ( node->type == RoadNode::Type::T_INTERSECTION )
	{
		if ( node->left_node )
		{
			rotation_degree = -90.f;
		}
		else if ( node->right_node )
		{
			rotation_degree = 90.f;
		}
	}

	Matrix t, r, t2, m;
	
	t.set_translation( -0.5f, -0.5f, 0.f );
	r.set_rotation_z( math::degree_to_radian( rotation_degree ) );
	t2.set_translation( 0.5f, 0.5f, 0.f );

	m = t * r * t2;

	model_->get_mesh()->add_vertex( Mesh::Vertex( node->back_left_pos.xyz(), Vector::Up.xyz(), ( Vector( 0.f, 0.f, 0.f, 1.f ) * m ).xy() ) ); // 0
	model_->get_mesh()->add_vertex( Mesh::Vertex( node->back_right_pos.xyz(), Vector::Up.xyz(), ( Vector( 1.f, 0.f, 0.f, 1.f ) * m ).xy() ) ); // 1

	// 分割数 ( 0 : 分割しない )
	const int subdivision_level = 3;

	auto* vertex_group = get_vertex_group_by_road_node( node );

	for ( auto n = 0; n < subdivision_level + 1; n++ )
	{
		const float a = static_cast< float >( n + 1 ) / static_cast< float >( subdivision_level + 1 );

		model_->get_mesh()->add_vertex( Mesh::Vertex( Vector::lerp( node->back_left_pos, node->front_left_pos, a ).xyz(), Vector::Up.xyz(), ( Vector( 0.f, a, 0.f, 1.f ) * m ).xy() ) ); // 2
		model_->get_mesh()->add_vertex( Mesh::Vertex( Vector::lerp( node->back_right_pos, node->front_right_pos, a ).xyz(), Vector::Up.xyz(), ( Vector( 1.f, a, 0.f, 1.f ) * m ).xy() ) ); // 3

		vertex_group->add_index( index_offset + ( n * 2 ) + 0 );
		vertex_group->add_index( index_offset + ( n * 2 ) + 2 );
		vertex_group->add_index( index_offset + ( n * 2 ) + 1 );

		vertex_group->add_index( index_offset + ( n * 2 ) + 1 );
		vertex_group->add_index( index_offset + ( n * 2 ) + 2 );
		vertex_group->add_index( index_offset + ( n * 2 ) + 3 );
	}
}

/**
 * 道路ノードから該当する VertexGroup を取得する
 */
CityGenerator::VertexGroup* CityGenerator::get_vertex_group_by_road_node( const RoadNode* node )
{
	std::array< VertexGroup*, 4 > vertex_group_map = {
		get_model()->get_mesh()->get_vertex_group_at( 0 ),	 // STRAIGHT,			///< 直線
		get_model()->get_mesh()->get_vertex_group_at( 1 ),	 // CROSS,				///< 十字路
		get_model()->get_mesh()->get_vertex_group_at( 0 ),	 // CURVE,				///< カーブ
		get_model()->get_mesh()->get_vertex_group_at( 2 ),	 // T_INTERSECTION,		///< T 字路
	};

	return vertex_group_map[ static_cast< int >( node->type ) ];
}



/**
 * デバッグ用のメッシュを生成する
 */
void CityGenerator::generate_debug_mesh()
{
	debug_model_->get_mesh()->clear_vertex_list();
	debug_model_->get_mesh()->get_vertex_group_at( 0 )->clear();

	for ( const auto& cp : road_control_point_list_ )
	{
		generate_debug_road_control_point_mesh( cp );
	}

	std::cout << "cp size : " << road_control_point_list_.size() << std::endl;
}

/**
 * デバッグ用のコントロールポイントのメッシュを生成する
 */
void CityGenerator::generate_debug_road_control_point_mesh( const RoadControlPoint& cp ) const
{
	const auto index_offset = debug_model_->get_mesh()->get_vertex_count();

	debug_model_->get_mesh()->add_vertex( Mesh::Vertex( ( cp.position + Vector( -1.f, 3.f, 0.f ) ).xyz(), { 0.f, 0.f, -1.f }, { 0.f, 1.f } ) ); // 0
	debug_model_->get_mesh()->add_vertex( Mesh::Vertex( ( cp.position + Vector( +1.f, 3.f, 0.f ) ).xyz(), { 0.f, 0.f, -1.f }, { 1.f, 1.f } ) ); // 1
	debug_model_->get_mesh()->add_vertex( Mesh::Vertex( ( cp.position + Vector( -1.f, 1.f, 0.f ) ).xyz(), { 0.f, 0.f, -1.f }, { 0.f, 0.f } ) ); // 2
	debug_model_->get_mesh()->add_vertex( Mesh::Vertex( ( cp.position + Vector( +1.f, 1.f, 0.f ) ).xyz(), { 0.f, 0.f, -1.f }, { 1.f, 0.f } ) ); // 3

	auto* vertex_group = debug_model_->get_mesh()->get_vertex_group_at( 0 );
	vertex_group->add_index( index_offset + 0 );
	vertex_group->add_index( index_offset + 3 );
	vertex_group->add_index( index_offset + 2 );

	vertex_group->add_index( index_offset + 0 );
	vertex_group->add_index( index_offset + 1 );
	vertex_group->add_index( index_offset + 3 );
}

} // namespace blue_sky
