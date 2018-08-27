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
}

/**
 * äXÇê∂ê¨Ç∑ÇÈ
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

	model_->get_mesh()->create_vertex_group();
	model_->get_mesh()->create_vertex_group();

	extend_road( control_point_, control_point_ - Vector( 4.f, 0.f, 0.f ), control_point_ + Vector( 4.f, 0.f, 0.f ), 0, 40, 5 );

	model_->get_mesh()->create_vertex_buffer();
	model_->get_mesh()->create_index_buffer();
}

/**
 * ìπòHÇêLÇŒÇ∑
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
 * ÉÅÉbÉVÉÖÇê∂ê¨Ç∑ÇÈ
 *
 */
void CityGenerator::generate_road_mesh()
{

}

} // namespace blue_sky
