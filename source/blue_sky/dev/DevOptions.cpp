#include "DevOptions.h"

#include <windows.h>

#include <cstdio>
#include <cstdlib>
#include <sstream>

namespace blue_sky::dev
{

namespace
{

/// 値が空でなければ返す（以前の GetEnvironmentVariableA は、空の値を「設定されていない」と同じに扱っていた）
std::optional< string_t > non_empty( const std::optional< string_t >& value )
{
	return value && ! value->empty() ? value : std::nullopt;
}

} // namespace

DevOptions DevOptions::read( const GetEnvironment& get_environment )
{
	DevOptions o;

	o.start_scene = non_empty( get_environment( "BLUE_SKY_START_SCENE" ) );
	o.start_stage = non_empty( get_environment( "BLUE_SKY_START_STAGE" ) );
	o.start_overlay = non_empty( get_environment( "BLUE_SKY_START_OVERLAY" ) );

	if ( const auto frames = non_empty( get_environment( "BLUE_SKY_CAPTURE_FRAMES" ) ) )
	{
		std::stringstream ss( * frames );

		for ( string_t item; std::getline( ss, item, ',' ); )
		{
			if ( ! item.empty() )
			{
				o.capture_frames.push_back( std::atoi( item.c_str() ) );
			}
		}
	}

	if ( const auto directory = non_empty( get_environment( "BLUE_SKY_CAPTURE_DIR" ) ) )
	{
		o.capture_directory = * directory;
	}

	// 値が空でも、設定されていれば保存する（以前と同じ）
	o.capture_glyph_atlas = get_environment( "BLUE_SKY_CAPTURE_GLYPH_ATLAS" ).has_value();

	if ( const auto size = non_empty( get_environment( "BLUE_SKY_DEBUG_DISPLAY_CHANGE" ) ) )
	{
		int w = 0, h = 0;

		if ( sscanf_s( size->c_str(), "%dx%d", & w, & h ) == 2 )
		{
			o.display_change_width = w;
			o.display_change_height = h;
		}
	}

	if ( const auto msec = non_empty( get_environment( "BLUE_SKY_DEBUG_FIXED_STEP_MSEC" ) ) )
	{
		unsigned long value = 0;

		if ( sscanf_s( msec->c_str(), "%lu", & value ) == 1 && value > 0 )
		{
			o.fixed_step_msec = value;
		}
	}

	if ( const auto range = non_empty( get_environment( "BLUE_SKY_DEBUG_FRAME_TIME" ) ) )
	{
		int first = 0, last = 0;

		if ( sscanf_s( range->c_str(), "%d,%d", & first, & last ) == 2 && last >= first )
		{
			o.frame_time_first = first;
			o.frame_time_last = last;
		}
	}

	// "シーン[:ステージ]" を , で並べる
	if ( const auto sequence = non_empty( get_environment( "BLUE_SKY_DEBUG_SCENE_SEQUENCE" ) ) )
	{
		std::stringstream ss( * sequence );

		for ( string_t item; std::getline( ss, item, ',' ); )
		{
			if ( item.empty() )
			{
				continue;
			}

			SceneRequest r { SceneRequest::Type::REPLACE_BASE };
			const auto colon = item.find( ':' );
			r.name = item.substr( 0, colon );
			r.stage_name = colon == string_t::npos ? string_t() : item.substr( colon + 1 );
			o.scene_sequence.push_back( r );
		}
	}

	return o;
}

DevOptions DevOptions::read_from_environment()
{
	return read( & get_environment_variable );
}

std::optional< string_t > get_environment_variable( const char_t* name )
{
	// 大きさを聞いてから読む（0 なら設定されていない。値が空のときは 1 = 終端の分）
	const DWORD size = GetEnvironmentVariableA( name, nullptr, 0 );

	if ( size == 0 )
	{
		return std::nullopt;
	}

	string_t value( size, '\0' );
	const DWORD length = GetEnvironmentVariableA( name, value.data(), size );
	value.resize( length < size ? length : 0 );

	return value;
}

} // namespace blue_sky::dev
