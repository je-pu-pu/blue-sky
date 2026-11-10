#include "StageLook.h"
#include "Action.h"

#include <blue_sky/graphics/RenderSettings.h>

#include <common/math.h>
#include <core/math.h>

namespace blue_sky::stage
{

namespace
{

/// 色は書いた成分だけ変える（以前の ss >> r >> g >> b >> a は、読めなかった所から後を前の値のままにした）
void set_components( core::Color& color, const std::vector< double >& v )
{
	float_t* c[] = { & color.r(), & color.g(), & color.b(), & color.a() };

	for ( size_t i = 0; i < v.size() && i < 4; ++i )
	{
		* c[ i ] = static_cast< float_t >( v[ i ] );
	}
}

/// 位置の足りない数は 0（以前と同じ）
core::Vector3 to_vector3( const std::vector< double >& v )
{
	auto at = [ & ] ( size_t i ) { return i < v.size() ? static_cast< float_t >( v[ i ] ) : 0.f; };
	return core::Vector3( at( 0 ), at( 1 ), at( 2 ) );
}

} // namespace

/// 初めの値は以前の GamePlayScene のコンストラクタと同じ
StageLook::StageLook()
	: light_position_( Vector3( 0.25f, 1.f, -1.f ), 0.1f )
	, ambient_color_( Color( 1.f, 1.f, 1.f, 1.f ), 0.02f )
	, shadow_color_( Color( 0.f, 0.f, 0.f, 1.f ), 0.02f )
	, shadow_paper_color_( Color( 0.9f, 0.9f, 0.9f, 1.f ), 0.02f )
{

}

void StageLook::apply( const Action& a, graphics::RenderSettings& settings )
{
	switch ( a.type )
	{
		case ActionType::SET:
		{
			if ( a.has( "sky_box" ) )
			{
				if ( a.text( "sky_box" ) == "none" )
				{
					settings.unset_sky_box();
				}
				else
				{
					settings.set_sky_box( ( "sky-box-" + a.text( "sky_box" ) ).c_str() );
				}
			}

			if ( a.has( "line_type" ) ) { settings.set_drawing_line_type( static_cast< int >( a.number( "line_type", 0.0 ) ) ); }
			if ( a.has( "paper_type" ) ) { settings.set_paper_texture_type( static_cast< int >( a.number( "paper_type", 0.0 ) ) ); }
			if ( a.has( "drawing_accent_scale" ) ) { drawing_accent_scale_ = static_cast< float_t >( a.number( "drawing_accent_scale", 0.0 ) ); }
			if ( a.has( "shading_enabled" ) ) { shading_enabled_ = a.flag( "shading_enabled", true ); }

			if ( auto v = a.numbers( "ambient_color" ) ) { set_components( ambient_color_.target_value(), * v ); ambient_color_.chase_full(); }
			if ( auto v = a.numbers( "shadow_color" ) ) { set_components( shadow_color_.target_value(), * v ); shadow_color_.chase_full(); }
			if ( auto v = a.numbers( "shadow_paper_color" ) ) { set_components( shadow_paper_color_.target_value(), * v ); shadow_paper_color_.chase_full(); }

			if ( auto v = a.numbers( "light_position" ) )
			{
				light_position_.target_value() = to_vector3( * v );
				light_position_.value() = light_position_.target_value();
			}

			break;
		}
		case ActionType::FADE:
		{
			// 速さは書いたときだけ変える（以前の set_*_color_target の 5 つ目の数）
			auto fade = [ & ] ( const char* key, common::chase_value< Color, float_t >& color )
			{
				if ( auto v = a.numbers( key ) )
				{
					set_components( color.target_value(), * v );

					if ( a.has( "speed" ) )
					{
						color.speed() = static_cast< float_t >( a.number( "speed", 0.0 ) );
					}
				}
			};

			fade( "ambient_color", ambient_color_ );
			fade( "shadow_color", shadow_color_ );
			fade( "shadow_paper_color", shadow_paper_color_ );

			if ( auto v = a.numbers( "light_position" ) )
			{
				light_position_.target_value() = to_vector3( * v );
			}

			break;
		}
		default:
		{
			break;
		}
	}
}

void StageLook::chase()
{
	light_position_.chase();
	ambient_color_.chase();
	shadow_color_.chase();
	shadow_paper_color_.chase();
}

void StageLook::apply_ambient_color( graphics::RenderSettings& settings ) const
{
	settings.set_ambient_color( ambient_color_.value() );
}

void StageLook::apply_shadow_and_accent( graphics::RenderSettings& settings, float_t peak_level ) const
{
	settings.set_shadow_color( shadow_color_.value() );
	settings.set_shadow_paper_color( shadow_paper_color_.value() );
	settings.set_drawing_accent( peak_level * drawing_accent_scale_ );
}

} // namespace blue_sky::stage
