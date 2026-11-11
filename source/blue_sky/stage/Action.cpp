#include "Action.h"

#include <cstdio>

namespace blue_sky::stage
{

string_t SourceLocation::to_string() const
{
	string_t s = file;

	if ( line > 0 )
	{
		s += ":" + std::to_string( line );

		if ( column > 0 )
		{
			s += ":" + std::to_string( column );
		}
	}

	return s;
}

namespace
{

struct ActionName
{
	ActionType	type;
	const char*	name;
};

const ActionName action_names[] =
{
	{ ActionType::SET,					"set" },
	{ ActionType::FADE,					"fade" },
	{ ActionType::PLAY_SOUND,			"play_sound" },
	{ ActionType::STOP_SOUND,			"stop_sound" },
	{ ActionType::STOP_ALL_SOUNDS,		"stop_all_sounds" },
	{ ActionType::LOAD_SOUND,			"load_sound" },
	{ ActionType::SPAWN,				"spawn" },
	{ ActionType::MOVE_TO,				"move_to" },
	{ ActionType::TURN_TO,				"turn_to" },
	{ ActionType::SET_BODY,				"set_body" },
	{ ActionType::SET_FLICKER,			"set_flicker" },
	{ ActionType::START_FLICKERING,		"start_flickering" },
	{ ActionType::PLAY_ANIMATION,		"play_animation" },
	{ ActionType::SET_ANIMATION_SPEED,	"set_animation_speed" },
	{ ActionType::SWITCH,				"switch" },
	{ ActionType::SET_MODE,				"set_mode" },
	{ ActionType::SET_VISIBLE,			"set_visible" },
	{ ActionType::BREAK_ANIMATION,		"break_animation" },
	{ ActionType::END_STAGE,			"end_stage" },
};

/// 色（1〜4 個。書いていない成分は前の値のまま。以前の set_*_color と同じ）
ArgSpec color( const char* key ) { return ArgSpec{ key, ArgKind::NUMBERS, 1, 4 }; }

/// 位置などの 3 つの数
ArgSpec vector3( const char* key, bool required = false ) { return ArgSpec{ key, ArgKind::NUMBERS, 3, 3, required }; }

ArgSpec target() { return ArgSpec{ "target", ArgKind::STRING, 0, 0, true }; }

} // namespace

const char* to_string( ActionType type )
{
	for ( const auto& n : action_names )
	{
		if ( n.type == type )
		{
			return n.name;
		}
	}

	return "?";
}

std::optional< ActionType > find_action_type( const string_t& name )
{
	for ( const auto& n : action_names )
	{
		if ( name == n.name )
		{
			return n.type;
		}
	}

	return std::nullopt;
}

const std::vector< ArgSpec >& get_arg_specs( ActionType type )
{
	// set の環境の値（[environment] と同じキー。設計 2.2）
	static const std::vector< ArgSpec > set =
	{
		{ "sky_box", ArgKind::STRING },
		{ "bgm", ArgKind::STRING },
		{ "bpm", ArgKind::NUMBER },
		{ "beat", ArgKind::NUMBER },
		{ "line_type", ArgKind::INT },
		{ "paper_type", ArgKind::INT },
		{ "drawing_accent_scale", ArgKind::NUMBER },
		{ "shading_enabled", ArgKind::BOOL },
		{ "balloon_sound_type", ArgKind::INT },
		color( "ambient_color" ),
		color( "shadow_color" ),
		color( "shadow_paper_color" ),
		vector3( "light_position" ),
	};

	static const std::vector< ArgSpec > fade =
	{
		color( "ambient_color" ),
		color( "shadow_color" ),
		color( "shadow_paper_color" ),
		vector3( "light_position" ),
		{ "speed", ArgKind::NUMBER },
	};

	static const std::vector< ArgSpec > play_sound = { { "sound", ArgKind::STRING, 0, 0, true }, { "force", ArgKind::BOOL }, { "loop", ArgKind::BOOL } };
	static const std::vector< ArgSpec > stop_sound = { { "sound", ArgKind::STRING, 0, 0, true } };
	static const std::vector< ArgSpec > none = { };
	static const std::vector< ArgSpec > load_sound = { { "name", ArgKind::STRING, 0, 0, true }, { "file", ArgKind::STRING }, { "replace", ArgKind::BOOL } };
	static const std::vector< ArgSpec > spawn = { { "model", ArgKind::STRING, 0, 0, true }, vector3( "position" ), vector3( "rotation" ) };
	static const std::vector< ArgSpec > move_to = { target(), vector3( "position", true ), { "speed", ArgKind::NUMBER } };
	static const std::vector< ArgSpec > turn_to = { target(), { "direction", ArgKind::NUMBER }, { "face", ArgKind::STRING }, { "speed", ArgKind::NUMBER } };
	static const std::vector< ArgSpec > set_body = { target(), { "mass", ArgKind::NUMBER }, vector3( "gravity" ), vector3( "angular_factor" ), { "kinematic", ArgKind::BOOL }, { "contact_response", ArgKind::BOOL } };
	static const std::vector< ArgSpec > set_flicker = { target(), { "scale", ArgKind::NUMBER } };
	static const std::vector< ArgSpec > play_animation = { target(), { "name", ArgKind::STRING, 0, 0, true }, { "force", ArgKind::BOOL }, { "loop", ArgKind::BOOL } };
	static const std::vector< ArgSpec > set_animation_speed = { target(), { "speed", ArgKind::NUMBER } };
	static const std::vector< ArgSpec > switch_ = { target(), { "state", ArgKind::STRING, 0, 0, true } };
	static const std::vector< ArgSpec > set_mode = { target(), { "mode", ArgKind::STRING, 0, 0, true } };
	static const std::vector< ArgSpec > set_visible = { target(), { "model", ArgKind::BOOL }, { "line", ArgKind::BOOL } };
	static const std::vector< ArgSpec > break_animation = { target(), { "broken", ArgKind::BOOL, 0, 0, true } };

	switch ( type )
	{
		case ActionType::SET:					return set;
		case ActionType::FADE:					return fade;
		case ActionType::PLAY_SOUND:			return play_sound;
		case ActionType::STOP_SOUND:			return stop_sound;
		case ActionType::STOP_ALL_SOUNDS:		return none;
		case ActionType::LOAD_SOUND:			return load_sound;
		case ActionType::SPAWN:					return spawn;
		case ActionType::MOVE_TO:				return move_to;
		case ActionType::TURN_TO:				return turn_to;
		case ActionType::SET_BODY:				return set_body;
		case ActionType::SET_FLICKER:			return set_flicker;
		case ActionType::START_FLICKERING:		return none;
		case ActionType::PLAY_ANIMATION:		return play_animation;
		case ActionType::SET_ANIMATION_SPEED:	return set_animation_speed;
		case ActionType::SWITCH:				return switch_;
		case ActionType::SET_MODE:				return set_mode;
		case ActionType::SET_VISIBLE:			return set_visible;
		case ActionType::BREAK_ANIMATION:		return break_animation;
		case ActionType::END_STAGE:				return none;
	}

	return none;
}

const ArgSpec* find_arg_spec( ActionType type, const string_t& key )
{
	for ( const auto& spec : get_arg_specs( type ) )
	{
		if ( key == spec.key )
		{
			return & spec;
		}
	}

	return nullptr;
}

string_t validate_action( const Action& action )
{
	for ( const auto& spec : get_arg_specs( action.type ) )
	{
		if ( spec.required && ! action.has( spec.key ) )
		{
			return string_t( spec.key ) + " がありません";
		}
	}

	switch ( action.type )
	{
		case ActionType::SET:
		case ActionType::FADE:
			if ( action.args.empty() || ( action.args.size() == 1 && ( action.has( "speed" ) || action.has( "beat" ) ) ) )
			{
				return "変える値がありません";
			}
			if ( action.has( "beat" ) && ! action.has( "bpm" ) )
			{
				return "beat は bpm と一緒に書きます";
			}
			break;

		case ActionType::SET_BODY:
			if ( action.args.size() < 2 )
			{
				return "変える値がありません";
			}
			break;

		case ActionType::TURN_TO:
			if ( action.has( "direction" ) == action.has( "face" ) )
			{
				return "direction と face のどちらか一方を書きます";
			}
			break;

		case ActionType::SWITCH:
			if ( const auto& s = action.text( "state" ); s != "on" && s != "off" && s != "broken" )
			{
				return "state は on・off・broken のどれかです（\"" + s + "\"）";
			}
			break;

		case ActionType::SET_MODE:
			if ( const auto& m = action.text( "mode" ); m != "float" && m != "rotation" && m != "patrol" )
			{
				return "mode は float・rotation・patrol のどれかです（\"" + m + "\"）";
			}
			break;

		default:
			break;
	}

	if ( action.is_timed && ( action.interval < 0.0 || action.count < 1 ) )
	{
		return "delay・every は 0 以上、count は 1 以上です";
	}

	return string_t();
}

double Action::number( const string_t& key, double default_value ) const
{
	auto i = args.find( key );
	const double* v = i != args.end() ? std::get_if< double >( & i->second ) : nullptr;
	return v ? * v : default_value;
}

bool Action::flag( const string_t& key, bool default_value ) const
{
	auto i = args.find( key );
	const bool* v = i != args.end() ? std::get_if< bool >( & i->second ) : nullptr;
	return v ? * v : default_value;
}

const string_t& Action::text( const string_t& key ) const
{
	static const string_t empty;

	auto i = args.find( key );
	const string_t* v = i != args.end() ? std::get_if< string_t >( & i->second ) : nullptr;
	return v ? * v : empty;
}

const std::vector< double >* Action::numbers( const string_t& key ) const
{
	auto i = args.find( key );
	return i != args.end() ? std::get_if< std::vector< double > >( & i->second ) : nullptr;
}

namespace
{

string_t number_text( double v )
{
	char buffer[ 64 ];
	std::snprintf( buffer, sizeof( buffer ), "%.17g", v );
	return buffer;
}

} // namespace

string_t describe( const Action& action )
{
	string_t s = to_string( action.type );

	for ( const auto& [ key, value ] : action.args )
	{
		s += " " + key + "=";

		if ( const bool* b = std::get_if< bool >( & value ) )
		{
			s += * b ? "true" : "false";
		}
		else if ( const double* d = std::get_if< double >( & value ) )
		{
			s += number_text( * d );
		}
		else if ( const string_t* t = std::get_if< string_t >( & value ) )
		{
			s += "\"" + * t + "\"";
		}
		else if ( const auto* v = std::get_if< std::vector< double > >( & value ) )
		{
			s += "[";

			for ( size_t i = 0; i < v->size(); ++i )
			{
				s += ( i ? ", " : "" ) + number_text( ( * v )[ i ] );
			}

			s += "]";
		}
	}

	if ( action.is_timed )
	{
		s += " every=" + number_text( action.interval ) + " count=" + std::to_string( action.count );
	}

	return s;
}

} // namespace blue_sky::stage
