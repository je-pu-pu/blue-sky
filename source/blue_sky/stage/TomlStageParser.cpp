#include "TomlStageParser.h"

#include <common/exception.h>

// toml++（ヘッダだけのライブラリ。source/lib/include/toml++。MIT）を読むのはこのファイルだけ
#include <toml++/toml.hpp>

#include <fstream>
#include <set>
#include <sstream>

namespace blue_sky::stage
{

namespace
{

/// 読みながら警告を溜める
class Reader
{
private:
	string_t	file_name_;
	StageData&	data_;

public:
	Reader( const string_t& file_name, StageData& data )
		: file_name_( file_name )
		, data_( data )
	{ }

	SourceLocation where( const toml::source_region& r ) const
	{
		return SourceLocation{ file_name_, static_cast< int >( r.begin.line ), static_cast< int >( r.begin.column ) };
	}

	void warn( const toml::source_region& r, const string_t& message )
	{
		data_.warnings.push_back( where( r ).to_string() + ": " + message );
	}

	/// 数（整数・小数）
	static std::optional< double > to_number( const toml::node& node )
	{
		if ( auto* i = node.as_integer() )
		{
			return static_cast< double >( i->get() );
		}

		if ( auto* f = node.as_floating_point() )
		{
			return f->get();
		}

		return std::nullopt;
	}

	/// 数の並び（min〜max 個。違えば std::nullopt）
	static std::optional< std::vector< double > > to_numbers( const toml::node& node, size_t min_count, size_t max_count )
	{
		const toml::array* a = node.as_array();

		if ( ! a || a->size() < min_count || a->size() > max_count )
		{
			return std::nullopt;
		}

		std::vector< double > values;

		for ( const auto& v : * a )
		{
			auto n = to_number( v );

			if ( ! n )
			{
				return std::nullopt;
			}

			values.push_back( * n );
		}

		return values;
	}

	/// 3 つの数
	static std::optional< Vector3d > to_vector3( const toml::node& node )
	{
		auto v = to_numbers( node, 3, 3 );

		if ( ! v )
		{
			return std::nullopt;
		}

		return Vector3d{ ( * v )[ 0 ], ( * v )[ 1 ], ( * v )[ 2 ] };
	}

	static string_t type_name( const toml::node& node )
	{
		switch ( node.type() )
		{
			case toml::node_type::table:			return "表";
			case toml::node_type::array:			return "配列";
			case toml::node_type::string:			return "文字列";
			case toml::node_type::integer:			return "整数";
			case toml::node_type::floating_point:	return "小数";
			case toml::node_type::boolean:			return "真偽";
			default:								return "日時";
		}
	}

	/// 動き 1 つ（読めなければ警告に残して std::nullopt）
	std::optional< Action > read_action( const toml::node& node )
	{
		const toml::table* t = node.as_table();

		if ( ! t )
		{
			warn( node.source(), "動きは { do = \"…\", … } の表で書きます（" + type_name( node ) + "）。飛ばします" );
			return std::nullopt;
		}

		Action action;
		action.where = where( t->source() );

		const toml::node* d = t->get( "do" );

		if ( ! d || ! d->is_string() )
		{
			warn( t->source(), "動きに do（文字列）がありません。飛ばします" );
			return std::nullopt;
		}

		const string_t name = d->as_string()->get();

		if ( auto type = find_action_type( name ) )
		{
			action.type = * type;
		}
		else
		{
			warn( d->source(), "知らない動き \"" + name + "\" を飛ばします" );
			return std::nullopt;
		}

		std::optional< double > delay, every;
		std::optional< int64_t > count;

		for ( const auto& [ k, v ] : * t )
		{
			const string_t key( k.str() );

			if ( key == "do" )
			{
				continue;
			}

			auto type_error = [ & ] ( const string_t& expected )
			{
				warn( v.source(), "動き " + name + " の " + key + " は" + expected + "です（" + type_name( v ) + "）。この動きを飛ばします" );
			};

			if ( key == "delay" || key == "every" )
			{
				auto n = to_number( v );

				if ( ! n )
				{
					type_error( "数" );
					return std::nullopt;
				}

				( key == "delay" ? delay : every ) = n;
				continue;
			}

			if ( key == "count" )
			{
				if ( ! v.is_integer() )
				{
					type_error( "整数" );
					return std::nullopt;
				}

				count = v.as_integer()->get();
				continue;
			}

			const ArgSpec* spec = find_arg_spec( action.type, key );

			if ( ! spec )
			{
				warn( k.source(), "動き " + name + " の知らないキー " + key + " を飛ばします" );
				continue;
			}

			switch ( spec->kind )
			{
				case ArgKind::BOOL:
					if ( ! v.is_boolean() ) { type_error( "真偽（true・false）" ); return std::nullopt; }
					action.args[ key ] = v.as_boolean()->get();
					break;

				case ArgKind::INT:
					if ( ! v.is_integer() ) { type_error( "整数" ); return std::nullopt; }
					action.args[ key ] = static_cast< double >( v.as_integer()->get() );
					break;

				case ArgKind::NUMBER:
					if ( auto n = to_number( v ) ) { action.args[ key ] = * n; }
					else { type_error( "数" ); return std::nullopt; }
					break;

				case ArgKind::STRING:
					if ( ! v.is_string() ) { type_error( "文字列" ); return std::nullopt; }
					action.args[ key ] = string_t( v.as_string()->get() );
					break;

				case ArgKind::NUMBERS:
					if ( auto n = to_numbers( v, spec->min_count, spec->max_count ) ) { action.args[ key ] = * n; }
					else
					{
						type_error( spec->min_count == spec->max_count
							? std::to_string( spec->min_count ) + " 個の数の配列"
							: std::to_string( spec->min_count ) + "〜" + std::to_string( spec->max_count ) + " 個の数の配列" );
						return std::nullopt;
					}
					break;
			}
		}

		// 時間を置く（delay = 秒 か every = 秒, count = 回数。以前の timer）
		if ( delay && ( every || count ) )
		{
			warn( t->source(), "動き " + name + " に delay と every・count を一緒には書けません。この動きを飛ばします" );
			return std::nullopt;
		}

		if ( count && ! every )
		{
			warn( t->source(), "動き " + name + " の count は every と一緒に書きます。この動きを飛ばします" );
			return std::nullopt;
		}

		if ( delay )
		{
			action.is_timed = true;
			action.interval = * delay;
			action.count = 1;
		}
		else if ( every )
		{
			action.is_timed = true;
			action.interval = * every;
			action.count = static_cast< int >( count.value_or( 1 ) );
		}

		if ( const string_t problem = validate_action( action ); ! problem.empty() )
		{
			warn( t->source(), "動き " + name + " : " + problem + "。この動きを飛ばします" );
			return std::nullopt;
		}

		return action;
	}

	/// 動きの配列
	std::vector< Action > read_actions( const toml::node& node, const string_t& what )
	{
		std::vector< Action > actions;
		const toml::array* a = node.as_array();

		if ( ! a )
		{
			warn( node.source(), what + " は動きの配列で書きます（" + type_name( node ) + "）。飛ばします" );
			return actions;
		}

		for ( const auto& v : * a )
		{
			if ( auto action = read_action( v ) )
			{
				actions.push_back( std::move( * action ) );
			}
		}

		return actions;
	}

	void read_stage( const toml::table& t )
	{
		for ( const auto& [ k, v ] : t )
		{
			const string_t key( k.str() );

			auto read_string = [ & ] ( string_t& out )
			{
				if ( v.is_string() ) { out = v.as_string()->get(); }
				else { warn( v.source(), "[stage] の " + key + " は文字列です（" + type_name( v ) + "）。飛ばします" ); }
			};

			if ( key == "collision" ) { read_string( data_.collision ); }
			else if ( key == "ground" ) { read_string( data_.ground ); }
			else if ( key == "bgm" ) { read_string( data_.bgm ); }
			else if ( key == "bgm_bpm" )
			{
				if ( auto n = to_number( v ) ) { data_.bgm_bpm = * n; }
				else { warn( v.source(), "[stage] の bgm_bpm は数です（" + type_name( v ) + "）。飛ばします" ); }
			}
			else if ( key == "bgm_loop" )
			{
				if ( v.is_boolean() ) { data_.bgm_loop = v.as_boolean()->get(); }
				else { warn( v.source(), "[stage] の bgm_loop は真偽です（" + type_name( v ) + "）。飛ばします" ); }
			}
			else
			{
				warn( k.source(), "[stage] の知らないキー " + key + " を飛ばします" );
			}
		}
	}

	void read_player( const toml::table& t )
	{
		PlayerDesc player;
		bool has_position = false;

		for ( const auto& [ k, v ] : t )
		{
			const string_t key( k.str() );

			if ( key == "position" )
			{
				if ( auto p = to_vector3( v ) ) { player.position = * p; has_position = true; }
				else { warn( v.source(), "[player] の position は 3 個の数の配列です。[player] を飛ばします" ); return; }
			}
			else if ( key == "camera_direction" )
			{
				if ( auto n = to_number( v ) ) { player.camera_direction = * n; }
				else { warn( v.source(), "[player] の camera_direction は数です（" + type_name( v ) + "）。飛ばします" ); }
			}
			else
			{
				warn( k.source(), "[player] の知らないキー " + key + " を飛ばします" );
			}
		}

		if ( ! has_position )
		{
			warn( t.source(), "[player] に position がありません。[player] を飛ばします" );
			return;
		}

		data_.player = player;
	}

	void read_goal( const toml::table& t )
	{
		for ( const auto& [ k, v ] : t )
		{
			const string_t key( k.str() );

			if ( key == "position" )
			{
				if ( auto p = to_vector3( v ) ) { data_.goal = * p; }
				else { warn( v.source(), "[goal] の position は 3 個の数の配列です。飛ばします" ); }
			}
			else
			{
				warn( k.source(), "[goal] の知らないキー " + key + " を飛ばします" );
			}
		}
	}

	/// 物体（positions のまとめ書きなら、その数だけ足す）
	void read_object( const toml::node& node )
	{
		static const std::set< string_t > typed_kinds = { "balloon", "rocket", "robot", "stone", "switch", "ladder", "medal", "girl", "umbrella" };

		const toml::table* t = node.as_table();

		if ( ! t )
		{
			warn( node.source(), "[[object]] は表で書きます。飛ばします" );
			return;
		}

		ObjectDesc o;
		o.where = where( t->source() );

		const toml::node* type = t->get( "type" );

		if ( ! type || ! type->is_string() )
		{
			warn( t->source(), "物体に type（文字列）がありません。飛ばします" );
			return;
		}

		o.type = type->as_string()->get();

		const bool is_static = o.type == "static";
		const bool is_area_switch = o.type == "area-switch";
		const bool is_translation = o.type == "translation";

		if ( ! is_static && ! is_area_switch && ! is_translation && ! typed_kinds.count( o.type ) )
		{
			warn( type->source(), "知らない種類の物体 \"" + o.type + "\" を飛ばします" );
			return;
		}

		std::vector< Vector3d > positions;
		bool has_size = false;

		auto skip = [ & ] ( const toml::node& v, const string_t& message )
		{
			warn( v.source(), "物体 " + o.type + " の" + message + "。この物体を飛ばします" );
		};

		for ( const auto& [ k, v ] : * t )
		{
			const string_t key( k.str() );

			if ( key == "type" )
			{
				continue;
			}
			else if ( key == "name" )
			{
				if ( v.is_string() )
				{
					o.names.push_back( v.as_string()->get() );
				}
				else if ( const toml::array* a = v.as_array(); a && a->is_homogeneous( toml::node_type::string ) )
				{
					for ( const auto& n : * a ) { o.names.push_back( n.as_string()->get() ); }
				}
				else
				{
					return skip( v, " name は文字列か文字列の配列です（" + type_name( v ) + "）" );
				}
			}
			else if ( key == "position" )
			{
				if ( auto p = to_vector3( v ) ) { positions.push_back( * p ); }
				else { return skip( v, " position は 3 個の数の配列です" ); }
			}
			else if ( key == "positions" )
			{
				const toml::array* a = v.as_array();

				if ( ! a || a->empty() )
				{
					return skip( v, " positions は位置（3 個の数の配列）の配列です" );
				}

				for ( const auto& p : * a )
				{
					if ( auto p3 = to_vector3( p ) ) { positions.push_back( * p3 ); }
					else { return skip( p, " positions は位置（3 個の数の配列）の配列です" ); }
				}
			}
			else if ( key == "direction" && ! is_static )
			{
				if ( auto n = to_number( v ) ) { o.direction = * n; }
				else { return skip( v, " direction は数です（" + type_name( v ) + "）" ); }
			}
			else if ( key == "model" && is_static )
			{
				if ( v.is_string() ) { o.model = v.as_string()->get(); }
				else { return skip( v, " model は文字列です（" + type_name( v ) + "）" ); }
			}
			else if ( key == "rotation" && is_static )
			{
				if ( auto r = to_vector3( v ) ) { o.rotation = * r; }
				else { return skip( v, " rotation は 3 個の数の配列です" ); }
			}
			else if ( key == "size" && ( is_area_switch || is_translation ) )
			{
				if ( auto s = to_vector3( v ) ) { o.size = * s; has_size = true; }
				else { return skip( v, " size は 3 個の数の配列です" ); }
			}
			else if ( key == "speed" && is_translation )
			{
				if ( auto n = to_number( v ) ) { o.speed = * n; }
				else { return skip( v, " speed は数です（" + type_name( v ) + "）" ); }
			}
			else if ( key == "on_turn_on" || key == "on_turn_off" )
			{
				const string_t event = key == "on_turn_on" ? "turn-on" : "turn-off";

				for ( auto& action : read_actions( v, key ) )
				{
					o.events.push_back( EventHandlerDesc{ event, std::move( action ) } );
				}
			}
			else
			{
				warn( k.source(), "物体 " + o.type + " の知らないキー " + key + " を飛ばします" );
			}
		}

		if ( positions.empty() )
		{
			warn( t->source(), "物体 " + o.type + " に position（か positions）がありません。この物体を飛ばします" );
			return;
		}

		if ( is_static && o.model.empty() )
		{
			warn( t->source(), "物体 static に model がありません。この物体を飛ばします" );
			return;
		}

		if ( is_area_switch && ! has_size )
		{
			warn( t->source(), "物体 area-switch に size がありません。この物体を飛ばします" );
			return;
		}

		// 動かない物の向きは回転の x（以前と同じ）
		if ( is_static )
		{
			o.direction = o.rotation[ 0 ];
		}

		for ( const auto& p : positions )
		{
			ObjectDesc copy = o;
			copy.position = p;
			data_.objects.push_back( std::move( copy ) );
		}
	}

	void read_event( const toml::node& node )
	{
		const toml::table* t = node.as_table();

		if ( ! t )
		{
			warn( node.source(), "[[event]] は表で書きます。飛ばします" );
			return;
		}

		const toml::node* when = t->get( "when" );

		if ( ! when || ! when->is_string() )
		{
			warn( t->source(), "[[event]] に when（文字列）がありません。飛ばします" );
			return;
		}

		if ( when->as_string()->get() != "start" )
		{
			warn( when->source(), "知らない条件 when = \"" + string_t( when->as_string()->get() ) + "\" の [[event]] を飛ばします（今は \"start\" だけ）" );
			return;
		}

		std::vector< Action > actions;

		for ( const auto& [ k, v ] : * t )
		{
			const string_t key( k.str() );

			if ( key == "when" )
			{
				continue;
			}
			else if ( key == "then" )
			{
				actions = read_actions( v, "then" );
			}
			else
			{
				warn( k.source(), "[[event]] の知らないキー " + key + " を飛ばします" );
			}
		}

		for ( auto& a : actions )
		{
			data_.start_actions.push_back( std::move( a ) );
		}
	}

	/// 名前の参照を調べる（読み終わったときの名前で。遊んでいる最中に名前は増えない）
	void check_references()
	{
		std::set< string_t > names = { "player", "goal" };
		std::map< string_t, SourceLocation > first;

		for ( const auto& o : data_.objects )
		{
			if ( o.type == "girl" )
			{
				names.insert( "girl" );
			}

			for ( const auto& name : o.names )
			{
				if ( auto i = first.find( name ); i != first.end() )
				{
					data_.warnings.push_back( o.where.to_string() + ": 名前 " + name + " は " + i->second.to_string() + " の物体にもあります（後の物体に付け替えます）" );
				}

				first[ name ] = o.where;
				names.insert( name );
			}
		}

		auto check = [ & ] ( const Action& a )
		{
			for ( const auto& [ key, value ] : a.args )
			{
				if ( is_reference_key( key ) && ! names.count( a.text( key ) ) )
				{
					data_.warnings.push_back( a.where.to_string() + ": 動き " + to_string( a.type ) + " の " + key + " = \"" + a.text( key ) + "\" という名前の物体がありません（何もしません）" );
				}
			}
		};

		for ( const auto& o : data_.objects )
		{
			for ( const auto& e : o.events )
			{
				check( e.action );
			}
		}

		for ( const auto& a : data_.start_actions )
		{
			check( a );
		}
	}
};

} // namespace

StageData TomlStageParser::parse_text( const string_t& text, const string_t& file_name )
{
	toml::table root;

	try
	{
		root = toml::parse( text, file_name );
	}
	catch ( const toml::parse_error& e )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( file_name + ":" + std::to_string( e.source().begin.line ) + ":" + std::to_string( e.source().begin.column ) + ": TOML の文法の誤り : " + string_t( e.description() ) );
	}

	StageData data;
	Reader reader( file_name, data );

	// 形式の版（読めない版は始められない）
	if ( const toml::node* format = root.get( "format" ) )
	{
		if ( ! format->is_integer() || format->as_integer()->get() != FORMAT_VERSION )
		{
			COMMON_THROW_EXCEPTION_MESSAGE( reader.where( format->source() ).to_string() + ": 読めない形式の版です（format = " + std::to_string( FORMAT_VERSION ) + " だけ読めます）" );
		}
	}
	else
	{
		reader.warn( root.source(), "format がありません（" + std::to_string( FORMAT_VERSION ) + " として読みます）" );
	}

	// 区画は決まった順に読む（表の中のキーの順は TOML では意味を持たないので、物体と動きの順は配列の順で決める）
	for ( const auto& [ k, v ] : root )
	{
		const string_t key( k.str() );

		if ( key == "format" || key == "object" || key == "event" )
		{
			continue;
		}

		const toml::table* t = v.as_table();

		if ( ( key == "stage" || key == "player" || key == "goal" ) && ! t )
		{
			reader.warn( v.source(), "[" + key + "] は表で書きます。飛ばします" );
		}
		else if ( key == "stage" )
		{
			reader.read_stage( * t );
		}
		else if ( key == "player" )
		{
			reader.read_player( * t );
		}
		else if ( key == "goal" )
		{
			reader.read_goal( * t );
		}
		else
		{
			reader.warn( k.source(), "知らないキー " + key + " を飛ばします" );
		}
	}

	for ( const char* array_key : { "object", "event" } )
	{
		const toml::node* node = root.get( array_key );

		if ( ! node )
		{
			continue;
		}

		const toml::array* a = node->as_array();

		if ( ! a )
		{
			reader.warn( node->source(), string_t( "[[" ) + array_key + "]] は表の配列で書きます。飛ばします" );
			continue;
		}

		for ( const auto& v : * a )
		{
			if ( string_t( array_key ) == "object" )
			{
				reader.read_object( v );
			}
			else
			{
				reader.read_event( v );
			}
		}
	}

	reader.check_references();

	return data;
}

StageData TomlStageParser::parse_file( const string_t& file_name )
{
	std::ifstream in( file_name, std::ios::binary );

	if ( ! in.good() )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( "ステージのファイル \"" + file_name + "\" を開けません" );
	}

	std::stringstream ss;
	ss << in.rdbuf();

	return parse_text( ss.str(), file_name );
}

} // namespace blue_sky::stage
