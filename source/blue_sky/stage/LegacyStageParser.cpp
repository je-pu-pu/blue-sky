#include "LegacyStageParser.h"

#include <common/exception.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <istream>
#include <optional>
#include <set>
#include <sstream>

namespace blue_sky::stage
{

namespace
{

/// 数を順に読む（以前の ss >> x >> y >> z と同じ : 読めなかった所から後は読まず、元の値のまま）
template< size_t N >
void read_numbers( std::istream& ss, std::array< double, N >& values )
{
	for ( auto& v : values )
	{
		double x = 0.0;

		if ( ! ( ss >> x ) )
		{
			return;
		}

		v = x;
	}
}

/// 残りの語（以前は黙って捨てていた）
string_t rest_of( std::istream& ss )
{
	string_t rest, word;

	while ( ss >> word )
	{
		rest += ( rest.empty() ? "" : " " ) + word;
	}

	return rest;
}

/**
 * 残りの語を空白 1 つでつなぐ（以前の event・exec 行の引数の作り方と同じ）
 *
 * 行末に空白があると、最後に空の語をつなぐので末尾に空白が 1 つ残る（以前と同じ。引数を読む側は空白を読み飛ばす）
 */
string_t join_params( std::istream& ss )
{
	string_t params;

	while ( ss.good() )
	{
		string_t s;
		ss >> s;

		if ( ! params.empty() )
		{
			params += " ";
		}

		params += s;
	}

	return params;
}

/// 引数を順に読む（以前の stringstream と同じ決まり : 読めなかった所から後は読まない）
class Tokens
{
private:
	std::vector< string_t >	words_;
	size_t					pos_ = 0;
	bool					failed_ = false;

public:
	explicit Tokens( const string_t& params )
	{
		std::istringstream ss( params );
		string_t w;

		while ( ss >> w )
		{
			words_.push_back( w );
		}
	}

	std::optional< string_t > word()
	{
		if ( failed_ || pos_ >= words_.size() )
		{
			failed_ = true;
			return std::nullopt;
		}

		return words_[ pos_++ ];
	}

	std::optional< double > number()
	{
		if ( failed_ || pos_ >= words_.size() )
		{
			failed_ = true;
			return std::nullopt;
		}

		// 語の全体が数として読めるときだけ（"5abc" は以前なら 5 を読んで後で止まる。同梱のステージには無い）
		std::istringstream ss( words_[ pos_ ] );
		double v = 0.0;

		if ( ! ( ss >> v ) || ! ss.eof() )
		{
			failed_ = true;
			return std::nullopt;
		}

		++pos_;
		return v;
	}

	std::vector< double > numbers( size_t n )
	{
		std::vector< double > values;

		for ( size_t i = 0; i < n; ++i )
		{
			auto v = number();

			if ( ! v )
			{
				break;
			}

			values.push_back( * v );
		}

		return values;
	}

	/// 0 / 1 だけ（以前の ss >> bool と同じ）
	std::optional< bool > boolean()
	{
		if ( failed_ || pos_ >= words_.size() || ( words_[ pos_ ] != "0" && words_[ pos_ ] != "1" ) )
		{
			failed_ = true;
			return std::nullopt;
		}

		return words_[ pos_++ ] == "1";
	}

	/// 残りの語（失敗した所から）
	string_t rest() const
	{
		string_t s;

		for ( size_t i = pos_; i < words_.size(); ++i )
		{
			s += ( s.empty() ? "" : " " ) + words_[ i ];
		}

		return s;
	}

	/// 残りの語の数
	size_t rest_count() const { return words_.size() - pos_; }

	/// 失敗を忘れて続きを読む
	void clear_failure() { failed_ = false; }
};

/**
 * 今の形式のコマンド 1 つを Action にする（tools/stage_convert/convert_stage.py の convert_action と同じ決まり）
 *
 * 書いてある値だけを引数にする（書いていない値は、行う側が以前と同じ既定の値を使う）。ただし、
 * 以前は 0 のままだった位置などの 3 つの数は、1 つでも書いてあれば足りない所を 0 で補う。
 *
 * @param warn 警告を足す
 * @return 動き（読めない・捨てる動きなら std::nullopt。理由は warn に出す）
 * @throw common::exception 知らないコマンド（以前と同じく、読み込むときに止める）
 */
std::optional< Action > convert_command( const string_t& name, const string_t& params, const SourceLocation& where, const std::function< void( const string_t& ) >& warn )
{
	Tokens t( params );
	Action a;
	a.where = where;

	auto done = [ & ] () -> std::optional< Action >
	{
		t.clear_failure();

		if ( t.rest_count() )
		{
			warn( name + " の余った引数を捨てます : " + t.rest() );
		}

		return a;
	};

	auto drop = [ & ] ( const string_t& reason ) -> std::optional< Action >
	{
		warn( "動きを捨てます : " + name + " " + params + "（" + reason + "）" );
		return std::nullopt;
	};

	auto pad3 = [] ( std::vector< double > v ) { v.resize( 3, 0.0 ); return v; };

	if ( name == "timer" )
	{
		auto interval = t.number();
		auto count = t.number();
		auto inner = t.word();

		if ( ! interval || ! count || ! inner )
		{
			return drop( "timer の間隔・回数・コマンドを読めない" );
		}

		auto action = convert_command( * inner, t.rest(), where, warn );

		if ( ! action )
		{
			return std::nullopt;
		}

		if ( action->is_timed )
		{
			return drop( "timer の中の timer" );
		}

		action->is_timed = true;
		action->interval = * interval;
		action->count = static_cast< int >( * count );

		return action;
	}

	struct SetKey { const char* command; const char* key; ArgKind kind; };

	static const SetKey simple_set[] =
	{
		{ "set_line_type", "line_type", ArgKind::INT },
		{ "set_paper_type", "paper_type", ArgKind::INT },
		{ "set_drawing_accent_scale", "drawing_accent_scale", ArgKind::NUMBER },
		{ "set_shading_enabled", "shading_enabled", ArgKind::BOOL },
		{ "set_balloon_sound_type", "balloon_sound_type", ArgKind::INT },
	};

	for ( const auto& s : simple_set )
	{
		if ( name != s.command )
		{
			continue;
		}

		a.type = ActionType::SET;

		if ( s.kind == ArgKind::BOOL )
		{
			auto v = t.boolean();

			if ( ! v )
			{
				return drop( "値を読めない" );
			}

			a.args[ s.key ] = * v;
		}
		else
		{
			auto v = t.number();

			if ( ! v || ( s.kind == ArgKind::INT && * v != std::floor( * v ) ) )
			{
				return drop( "値を読めない" );
			}

			a.args[ s.key ] = * v;
		}

		return done();
	}

	struct ColorKey { const char* prefix; const char* key; };

	static const ColorKey colors[] = { { "ambient", "ambient_color" }, { "shadow", "shadow_color" }, { "shadow_paper", "shadow_paper_color" } };

	for ( const auto& c : colors )
	{
		const string_t set_name = string_t( "set_" ) + c.prefix + "_color";

		if ( name != set_name && name != set_name + "_target" )
		{
			continue;
		}

		const auto values = t.numbers( 4 );

		if ( values.empty() )
		{
			return drop( "色を読めない" );
		}

		a.type = name == set_name ? ActionType::SET : ActionType::FADE;
		a.args[ c.key ] = values;

		if ( a.type == ActionType::FADE && values.size() == 4 )
		{
			if ( auto speed = t.number() )
			{
				a.args[ "speed" ] = * speed;
			}
		}

		return done();
	}

	if ( name == "set_light_position" || name == "set_light_position_target" )
	{
		const auto p = t.numbers( 3 );

		if ( p.empty() )
		{
			return drop( "位置を読めない" );
		}

		a.type = name == "set_light_position" ? ActionType::SET : ActionType::FADE;
		a.args[ "light_position" ] = pad3( p );
		return done();
	}

	if ( name == "change_sky_box" )
	{
		auto v = t.word();

		if ( ! v )
		{
			return drop( "名前が無い" );
		}

		if ( auto ext = t.word() )
		{
			warn( "change_sky_box の 2 語目を捨てます : " + * ext + "（以前も使っていなかった）" );
		}

		a.type = ActionType::SET;
		a.args[ "sky_box" ] = * v;
		return done();
	}

	if ( name == "change_bgm" || name == "stop_sound" )
	{
		// 引数の全体が名前（以前と同じ）
		Tokens all( params );
		const string_t whole = all.rest();

		if ( whole.empty() )
		{
			return drop( "名前が無い" );
		}

		a.type = name == "change_bgm" ? ActionType::SET : ActionType::STOP_SOUND;
		a.args[ name == "change_bgm" ? "bgm" : "sound" ] = whole;
		return a;
	}

	if ( name == "change_bpm" )
	{
		auto bpm = t.number();

		if ( ! bpm )
		{
			return drop( "値を読めない" );
		}

		a.type = ActionType::SET;
		a.args[ "bpm" ] = * bpm;

		if ( auto beat = t.number() )
		{
			a.args[ "beat" ] = * beat;
		}

		return done();
	}

	if ( name == "load_sound" )
	{
		auto n = t.word();

		if ( ! n )
		{
			return drop( "名前が無い" );
		}

		a.type = ActionType::LOAD_SOUND;
		a.args[ "name" ] = * n;

		if ( auto f = t.word() )
		{
			a.args[ "file" ] = * f;

			if ( auto r = t.boolean() )
			{
				a.args[ "replace" ] = * r;
			}
		}

		return done();
	}

	if ( name == "play_sound" )
	{
		auto n = t.word();

		if ( ! n )
		{
			return drop( "名前が無い" );
		}

		a.type = ActionType::PLAY_SOUND;
		a.args[ "sound" ] = * n;

		if ( auto f = t.boolean() )
		{
			a.args[ "force" ] = * f;

			if ( auto l = t.boolean() )
			{
				a.args[ "loop" ] = * l;
			}
		}

		return done();
	}

	if ( name == "stop_all_sound" ) { a.type = ActionType::STOP_ALL_SOUNDS; return done(); }
	if ( name == "end_by_last_switch" ) { a.type = ActionType::END_STAGE; return done(); }
	if ( name == "player.start_flickering" ) { a.type = ActionType::START_FLICKERING; return done(); }

	if ( name == "game_object.create" )
	{
		auto model = t.word();

		if ( ! model )
		{
			return drop( "モデルが無い" );
		}

		a.type = ActionType::SPAWN;
		a.args[ "model" ] = * model;

		if ( const auto p = t.numbers( 3 ); ! p.empty() )
		{
			a.args[ "position" ] = pad3( p );

			if ( const auto r = t.numbers( 3 ); ! r.empty() )
			{
				a.args[ "rotation" ] = pad3( r );
			}
		}

		return done();
	}

	const string_t prefix = "game_object.";

	if ( name.compare( 0, prefix.size(), prefix ) != 0 )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( where.to_string() + ": event handler not found. ( " + name + " )" );
	}

	const string_t op = name.substr( prefix.size() );

	static const char* const known_ops[] = { "set_mass", "set_gravity", "set_angular_factor", "set_kinematic", "set_no_contact_response", "set_flicker_scale", "set_target_location", "set_target_direction", "set_target_direction_object", "play_animation", "set_animation_speed", "action" };

	if ( std::find_if( std::begin( known_ops ), std::end( known_ops ), [ & ] ( const char* k ) { return op == k; } ) == std::end( known_ops ) )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( where.to_string() + ": event handler not found. ( " + name + " )" );
	}

	auto target = t.word();

	if ( ! target )
	{
		return drop( "物体の名前が無い" );
	}

	a.args[ "target" ] = * target;

	if ( op == "set_mass" )
	{
		a.type = ActionType::SET_BODY;
		a.args[ "mass" ] = t.number().value_or( 0.0 );
		return done();
	}

	if ( op == "set_gravity" || op == "set_angular_factor" )
	{
		a.type = ActionType::SET_BODY;
		a.args[ op == "set_gravity" ? "gravity" : "angular_factor" ] = pad3( t.numbers( 3 ) );
		return done();
	}

	if ( op == "set_kinematic" )
	{
		a.type = ActionType::SET_BODY;
		a.args[ "kinematic" ] = t.boolean().value_or( false );
		return done();
	}

	if ( op == "set_no_contact_response" )
	{
		a.type = ActionType::SET_BODY;
		a.args[ "contact_response" ] = ! t.boolean().value_or( false );
		return done();
	}

	if ( op == "set_flicker_scale" )
	{
		a.type = ActionType::SET_FLICKER;

		if ( auto v = t.number() )
		{
			a.args[ "scale" ] = * v;
		}

		return done();
	}

	if ( op == "set_target_location" )
	{
		a.type = ActionType::MOVE_TO;

		const auto p = t.numbers( 3 );
		a.args[ "position" ] = pad3( p );

		if ( p.size() == 3 )
		{
			if ( auto s = t.number() )
			{
				a.args[ "speed" ] = * s;
			}
		}

		return done();
	}

	if ( op == "set_target_direction" )
	{
		a.type = ActionType::TURN_TO;

		auto r = t.number();
		a.args[ "direction" ] = r.value_or( 0.0 );

		if ( r )
		{
			if ( auto s = t.number() )
			{
				a.args[ "speed" ] = * s;
			}
		}

		return done();
	}

	if ( op == "set_target_direction_object" )
	{
		auto face = t.word();

		if ( ! face )
		{
			return drop( "向く先が無い" );
		}

		a.type = ActionType::TURN_TO;
		a.args[ "face" ] = * face;

		if ( auto s = t.number() )
		{
			a.args[ "speed" ] = * s;
		}

		return done();
	}

	if ( op == "play_animation" )
	{
		auto n = t.word();

		if ( ! n )
		{
			return drop( "アニメーションの名前が無い" );
		}

		a.type = ActionType::PLAY_ANIMATION;
		a.args[ "name" ] = * n;

		if ( auto f = t.boolean() )
		{
			a.args[ "force" ] = * f;

			if ( auto l = t.boolean() )
			{
				a.args[ "loop" ] = * l;
			}
		}

		return done();
	}

	if ( op == "set_animation_speed" )
	{
		a.type = ActionType::SET_ANIMATION_SPEED;

		if ( auto v = t.number() )
		{
			a.args[ "speed" ] = * v;
		}

		return done();
	}

	// action（物体ごとの動き。以前は文字列のまま物体に渡していた）
	const string_t text = t.rest();

	if ( text == "turn-on" || text == "turn-off" || text == "break" )
	{
		a.type = ActionType::SWITCH;
		a.args[ "state" ] = string_t( text == "turn-on" ? "on" : text == "turn-off" ? "off" : "broken" );
		return a;
	}

	if ( text == "float" || text == "rotation" || text == "patrol" )
	{
		a.type = ActionType::SET_MODE;
		a.args[ "mode" ] = text;
		return a;
	}

	if ( text == "break_animation 1" || text == "break_animation 0" )
	{
		a.type = ActionType::BREAK_ANIMATION;
		a.args[ "broken" ] = text.back() == '1';
		return a;
	}

	Tokens v( text );

	if ( v.word() == string_t( "set_visible" ) )
	{
		a.type = ActionType::SET_VISIBLE;

		if ( auto m = v.boolean() )
		{
			a.args[ "model" ] = * m;

			if ( auto l = v.boolean() )
			{
				a.args[ "line" ] = * l;
			}
		}

		v.clear_failure();

		if ( v.rest_count() )
		{
			warn( "set_visible の余った引数を捨てます : " + v.rest() );
		}

		return a;
	}

	return drop( "知らない物体の動き action " + text );
}

/**
 * 見つからない名前を指す動きを捨てる（警告に残す）
 *
 * 名前は読み込むときにしか付かない（遊んでいる最中に作る物体には名前が無い）ので、読み終わったときに無い名前は
 * 遊んでいる最中にも見つからず、以前も何もしなかった。変換ツールも同じ動きを捨てる（2-1 の switch-j）
 */
void drop_unresolved_references( StageData& data )
{
	std::set< string_t > names = { "player", "goal" };

	for ( const auto& o : data.objects )
	{
		if ( o.type == "girl" )
		{
			names.insert( "girl" );
		}

		names.insert( o.names.begin(), o.names.end() );
	}

	auto unresolved = [ & ] ( const Action& a )
	{
		for ( const char* key : { "target", "face" } )
		{
			if ( a.has( key ) && ! names.count( a.text( key ) ) )
			{
				data.warnings.push_back( a.where.to_string() + ": 動きを捨てます : " + describe( a ) + "（" + a.text( key ) + " という名前の物体はありません。以前も何もしなかった）" );
				return true;
			}
		}

		return false;
	};

	for ( auto& o : data.objects )
	{
		o.events.erase( std::remove_if( o.events.begin(), o.events.end(), [ & ] ( const EventHandlerDesc& e ) { return unresolved( e.action ); } ), o.events.end() );
	}

	data.start_actions.erase( std::remove_if( data.start_actions.begin(), data.start_actions.end(), unresolved ), data.start_actions.end() );
}

} // namespace

StageData LegacyStageParser::parse_text( const string_t& text, const string_t& file_name )
{
	std::istringstream in( text );
	return parse( in, file_name );
}

StageData LegacyStageParser::parse( std::istream& in, const string_t& file_name )
{
	StageData data;

	ObjectDesc* last_object = nullptr;		///< 直前に作った物体（event・name 行の対象）
	int line_number = 0;

	auto where = [ & ] { return SourceLocation{ file_name, line_number, 0 }; };
	auto warn = [ & ] ( const string_t& message ) { data.warnings.push_back( where().to_string() + ": " + message ); };

	auto warn_rest = [ & ] ( std::istream& ss, const string_t& command )
	{
		ss.clear();		// 数として読めなかった語も、捨てた語として知らせる

		if ( const string_t rest = rest_of( ss ); ! rest.empty() )
		{
			warn( command + " 行の余った引数を捨てます : " + rest );
		}
	};

	auto add_object = [ & ] ( ObjectDesc&& o ) -> ObjectDesc*
	{
		o.where = where();
		data.objects.push_back( std::move( o ) );
		return & data.objects.back();
	};

	while ( in.good() )
	{
		string_t line;
		std::getline( in, line );
		++line_number;

		std::istringstream ss( line );
		string_t command;
		ss >> command;

		if ( command.empty() || command[ 0 ] == '#' )
		{
			// 空行・コメント（以前はステージの設定として読み、設定の側が読み飛ばしていた）
		}
		else if ( command == "bgm" )
		{
			ss >> data.bgm;

			// 2 語目（繰り返すか）は以前も読むだけで使っていなかった
			bool loop = true;
			ss >> loop;

			if ( ss.good() )
			{
				if ( double bpm = 0.0; ss >> bpm )
				{
					data.bgm_bpm = bpm;
				}
			}
		}
		else if ( command == "collision" )
		{
			ss >> data.collision;
			warn_rest( ss, command );
		}
		else if ( command == "player" )
		{
			PlayerDesc player;
			read_numbers( ss, player.position );

			// 位置の後に何か書いてあればカメラの向き（読めなければ 0。以前と同じ）
			if ( ! ss.eof() )
			{
				double r = 0.0;
				ss >> r;
				player.camera_direction = r;
			}

			data.player = player;
		}
		else if ( command == "goal" )
		{
			Vector3d goal = { };
			read_numbers( ss, goal );
			warn_rest( ss, command );

			data.goal = goal;
		}
		else if ( command == "ground" )
		{
			ss >> data.ground;
			warn_rest( ss, command );
		}
		else if ( command == "far-billboards" )
		{
			// 以前は読み込んでも、ステージを読んだ後で既定の遠景（far-billboards-1）に上書きされ、効いていなかった
			warn( "far-billboards 行は効かないので読みません : " + rest_of( ss ) );
		}
		else if ( command == "object" || command == "static-object" || command == "dynamic-object" )
		{
			// 以前の ObjectFactory::create_static_object( ss ) : モデル x y z rx ry rz（dynamic-object も中身は同じ）
			ObjectDesc o;
			o.type = "static";
			ss >> o.model;
			read_numbers( ss, o.position );
			read_numbers( ss, o.rotation );
			o.direction = o.rotation[ 0 ];
			warn_rest( ss, command );

			last_object = add_object( std::move( o ) );
		}
		else if ( command == "translation-object" )
		{
			ObjectDesc o;
			o.type = "translation";
			read_numbers( ss, o.position );
			read_numbers( ss, o.size );

			if ( double speed = 0.0; ss >> speed )
			{
				o.speed = speed;
			}

			warn_rest( ss, command );

			last_object = add_object( std::move( o ) );
		}
		else if ( command == "girl" || command == "robot" || command == "balloon" || command == "medal" || command == "ladder" || command == "rocket" || command == "umbrella" || command == "stone" || command == "switch" )
		{
			ObjectDesc o;
			o.type = command;
			read_numbers( ss, o.position );

			if ( double r = 0.0; ss >> r )
			{
				o.direction = r;
			}

			warn_rest( ss, command );

			last_object = add_object( std::move( o ) );
		}
		else if ( command == "area-switch" )
		{
			ObjectDesc o;
			o.type = command;
			read_numbers( ss, o.position );
			read_numbers( ss, o.size );

			if ( double r = 0.0; ss >> r )
			{
				o.direction = r;
			}

			warn_rest( ss, command );

			last_object = add_object( std::move( o ) );
		}
		else if ( command == "event" || command == "exec" )
		{
			string_t event_name;

			if ( command == "event" )
			{
				ss >> event_name;
			}

			string_t action_name;
			ss >> action_name;
			const string_t params = join_params( ss );

			// 読み込むときに種類と型の付いた動きにする（知らないコマンドは以前と同じく例外。読めない動きは警告にして捨てる）
			auto converted = convert_command( action_name, params, where(), warn );

			if ( ! converted )
			{
				continue;
			}

			Action action = std::move( * converted );

			if ( command == "event" )
			{
				if ( ! last_object )
				{
					COMMON_THROW_EXCEPTION_MESSAGE( where().to_string() + ": event object not found." );
				}

				last_object->events.push_back( EventHandlerDesc{ event_name, std::move( action ) } );
			}
			else
			{
				data.start_actions.push_back( std::move( action ) );
			}
		}
		else if ( command == "name" )
		{
			if ( ! last_object )
			{
				COMMON_THROW_EXCEPTION_MESSAGE( where().to_string() + ": naming object not found." );
			}

			string_t name;
			ss >> name;
			last_object->names.push_back( name );

			// 2 語目からは以前も読んでいなかった（2-0 の "name switch switch-a" は "switch" という名前になる）
			warn_rest( ss, command );
		}
		else if ( command == "bgm.loop" )
		{
			// 以前はステージの設定として読み、BGM を鳴らすときに "bgm.loop" を読んでいた（0 / 1 以外は読めず既定の繰り返す）
			string_t value;
			ss >> value;

			if ( value == "0" || value == "1" )
			{
				data.bgm_loop = value == "1";
			}
			else
			{
				warn( "bgm.loop の値 \"" + value + "\" を読めません（0 か 1）" );
			}
		}
		else
		{
			// 以前はステージの設定として読んでいたが、bgm.loop のほかに読む所が無い（sky-box・video.shadow-map-enabled など）
			warn( "どこも読まない行を捨てます : " + line );
		}
	}

	drop_unresolved_references( data );

	return data;
}

} // namespace blue_sky::stage
