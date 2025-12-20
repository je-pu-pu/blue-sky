#include "Shader.h"
#include <common/exception.h>
#include <common/serialize.h>
#include <imgui.h>
#include <string>

#include <core/graphics/Direct3D11/Texture.h>
#include <portable-file-dialogs.h>

#include <blue_sky/GameMain.h>
#include <blue_sky/graphics/GraphicsManager.h>

namespace core::graphics
{

int_t calc_digit_number_after_decimal_point( float_t value )
{
	auto digit_number_after_decimal_point = 0;
	
	for ( auto v = value; v != std::floor( v ); v *= 10.f )
	{
		digit_number_after_decimal_point++;
	}

	return digit_number_after_decimal_point;
}

/**
 * シェーダーのパラメーターを操作するための GUI を描画する
 */
void Shader::render_parameter_gui()
{
	ImGui::Begin( get_name() );

	if ( get_parameter_info_list() )
	{
		for ( const auto& p : * get_parameter_info_list() )
		{
			// const auto max_digit = std::max( calc_digit_number_after_decimal_point( p.min ), calc_digit_number_after_decimal_point( p.max ) );
			// std::pow( 0.1f, max_digit );

			const auto step = ( p.min == std::numeric_limits< float_t >::lowest() || p.max == std::numeric_limits< float_t >::max() ) ? 0.1f : ( p.max - p.min ) * 0.001f;

			// const auto s = common::serialize( step );
			// const auto l = s.length();

			// const auto format = string_t( "%." ) + std::to_string( calc_digit_number_after_decimal_point( step ) ) + "f";
			const auto format = string_t( "%f" );

			switch ( p.type )
			{
				case ParameterType::INT:
				{
					auto n = get_int( p.name.c_str() );
					if ( ImGui::DragInt( p.name.c_str(), & n, 1, static_cast< int >( p.min ), static_cast< int >( p.max ) ) )
					{
						set_int( p.name.c_str(), n );
					}
					break;
				}
				case ParameterType::FLOAT:
				{
					auto f = get_float( p.name.c_str() );
					if ( ImGui::DragFloat( p.name.c_str(), & f, step, p.min, p.max, format.c_str() ) )
					{
						set_float( p.name.c_str(), f );
					}
					break;
				}
				case ParameterType::VECTOR:
				{
					auto v = get_vector( p.name.c_str() );
					if ( ImGui::DragFloat3( p.name.c_str(), reinterpret_cast< float* >( & v ), step, p.min, p.max, format.c_str() ) )
					{
						set_vector( p.name.c_str(), v );
					}
					break;
				}
				case ParameterType::COLOR:
				{
					auto c = get_color( p.name.c_str() );
					if ( ImGui::ColorEdit4( p.name.c_str(), reinterpret_cast< float* >( & c ) ) )
					{
						set_color( p.name.c_str(), c );
					}
					break;
				}
				default:
					COMMON_THROW_EXCEPTION_MESSAGE( "Shader::render_parameter_gui() ParameterType not handled." );
			}
		}
	}

	for ( int n = 0; const auto* t = get_texture_at( n ); n++ )
	{
		auto texture = dynamic_cast< const core::graphics::direct_3d_11::Texture* >( t );

		if ( ! texture )
		{
			break;
		}

		if ( ImGui::ImageButton( texture->get_shader_resource_view(), ImVec2( 64, 64 ) ) )
		{
			 auto file = pfd::open_file( "image file", "./", { "Image files (.png)", "*.png" } );

			 for ( auto const &name : file.result() )
			 {
				 auto* new_texture = blue_sky::GameMain::get_instance()->get_graphics_manager()->load_texture( name.c_str() );

				 if ( ! new_texture )
				 {
					 continue;
				 }

				 set_texture_at( n, new_texture );
			 }
		}
	}

	ImGui::End();
}

} // namespace core::graphics
