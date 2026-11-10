#include "StoryTextParser.h"

#include <common/exception.h>

// toml++（ヘッダだけのライブラリ。source/lib/include/toml++。MIT）
#include <toml++/toml.hpp>

#include <fstream>
#include <sstream>

namespace blue_sky::stage
{

namespace
{

/// 読みながら警告を溜める
class Reader
{
private:
	string_t		file_name_;
	StoryTextData&	data_;

public:
	Reader( const string_t& file_name, StoryTextData& data )
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

	/// 型の違いの警告（そのキーを飛ばす）
	void warn_type( const toml::node& v, const string_t& key, const string_t& expected )
	{
		warn( v.source(), key + " は" + expected + "で書きます（" + type_name( v ) + "）。飛ばします" );
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

	/// N 個の数（N == 1 なら数 1 つ、ほかは N 個の数の配列）
	template< size_t N >
	static std::optional< std::array< double, N > > to_numbers( const toml::node& node )
	{
		std::array< double, N > values { };

		if constexpr ( N == 1 )
		{
			auto n = to_number( node );

			if ( ! n )
			{
				return std::nullopt;
			}

			values[ 0 ] = * n;
		}
		else
		{
			const toml::array* a = node.as_array();

			if ( ! a || a->size() != N )
			{
				return std::nullopt;
			}

			for ( size_t i = 0; i < N; ++i )
			{
				auto n = to_number( * a->get( i ) );

				if ( ! n )
				{
					return std::nullopt;
				}

				values[ i ] = * n;
			}
		}

		return values;
	}

	/// 文字列のキー
	std::optional< string_t > read_string( const toml::node& v, const string_t& key )
	{
		if ( ! v.is_string() )
		{
			warn_type( v, key, "文字列" );
			return std::nullopt;
		}

		return v.as_string()->get();
	}

	/// { from = …, to = …, speed = … }
	template< size_t N >
	std::optional< ChaseDesc< N > > read_chase( const toml::node& node, const string_t& key )
	{
		const string_t expected = N == 1 ? "数" : std::to_string( N ) + " 個の数の配列";
		const toml::table* t = node.as_table();

		if ( ! t )
		{
			warn_type( node, key, "{ from = …, to = …, speed = … } の表" );
			return std::nullopt;
		}

		ChaseDesc< N > chase;

		for ( const auto& [ k, v ] : * t )
		{
			const string_t name( k.str() );
			std::optional< std::array< double, N > >* target = nullptr;

			if ( name == "from" )
			{
				target = & chase.from;
			}
			else if ( name == "to" )
			{
				target = & chase.to;
			}
			else if ( name == "speed" )
			{
				target = & chase.speed;
			}
			else
			{
				warn( k.source(), key + " の知らないキー " + name + " を飛ばします" );
				continue;
			}

			if ( auto values = to_numbers< N >( v ) )
			{
				* target = * values;
			}
			else
			{
				warn_type( v, key + "." + name, expected );
			}
		}

		return chase;
	}

	/// [[layer]] 1 つ
	void read_layer( const toml::node& node )
	{
		const toml::table* t = node.as_table();

		if ( ! t )
		{
			warn( node.source(), "[[layer]] は表で書きます。飛ばします" );
			return;
		}

		StoryLayerDesc layer;
		layer.where = where( t->source() );

		const toml::node* image = t->get( "image" );

		if ( ! image || ! image->is_string() )
		{
			warn( t->source(), "[[layer]] に image（文字列）がありません。この重なりを飛ばします" );
			return;
		}

		layer.image = image->as_string()->get();

		for ( const auto& [ k, v ] : * t )
		{
			const string_t key( k.str() );

			if ( key == "image" )
			{
				continue;
			}
			else if ( key == "src_rect" )
			{
				const toml::array* a = v.as_array();
				bool ok = a && a->size() == 4;
				std::array< int, 4 > rect { };

				for ( size_t i = 0; ok && i < 4; ++i )
				{
					const toml::node* n = a->get( i );
					ok = n->is_integer();

					if ( ok )
					{
						rect[ i ] = static_cast< int >( n->as_integer()->get() );
					}
				}

				if ( ok )
				{
					layer.src_rect = rect;
				}
				else
				{
					warn_type( v, key, "4 個の整数の配列（左・上・右・下）" );
				}
			}
			else if ( key == "translation" )
			{
				layer.translation = read_chase< 2 >( v, key );
			}
			else if ( key == "rotation" )
			{
				layer.rotation = read_chase< 1 >( v, key );
			}
			else if ( key == "scale" )
			{
				layer.scale = read_chase< 1 >( v, key );
			}
			else if ( key == "color" )
			{
				layer.color = read_chase< 4 >( v, key );
			}
			else
			{
				warn( k.source(), "[[layer]] の知らないキー " + key + " を飛ばします" );
			}
		}

		data_.layers.push_back( std::move( layer ) );
	}

	/// 色（0xRRGGBBAA の整数）
	std::optional< uint32_t > read_color( const toml::node& v, const string_t& key )
	{
		if ( ! v.is_integer() || v.as_integer()->get() < 0 || v.as_integer()->get() > 0xFFFFFFFFll )
		{
			warn_type( v, key, "0xRRGGBBAA の整数" );
			return std::nullopt;
		}

		return static_cast< uint32_t >( v.as_integer()->get() );
	}

	/// [text]
	void read_text( const toml::table& t )
	{
		for ( const auto& [ k, v ] : t )
		{
			const string_t key( k.str() );

			if ( key == "color" )
			{
				data_.text_color = read_color( v, "text.color" );
			}
			else if ( key == "border_color" )
			{
				data_.text_border_color = read_color( v, "text.border_color" );
			}
			else if ( key == "body" )
			{
				if ( auto s = read_string( v, "text.body" ) )
				{
					data_.text = * s;
				}
			}
			else
			{
				warn( k.source(), "[text] の知らないキー " + key + " を飛ばします" );
			}
		}
	}

	/// bgm = { file = "…", volume = … }
	void read_bgm( const toml::node& v )
	{
		const toml::table* t = v.as_table();

		if ( ! t )
		{
			warn_type( v, "bgm", "{ file = \"…\", volume = … } の表" );
			return;
		}

		const toml::node* file = t->get( "file" );

		if ( ! file || ! file->is_string() )
		{
			warn( t->source(), "bgm に file（文字列）がありません。飛ばします" );
			return;
		}

		StoryBgmDesc bgm;
		bgm.file = file->as_string()->get();

		for ( const auto& [ k, value ] : * t )
		{
			const string_t key( k.str() );

			if ( key == "file" )
			{
				continue;
			}
			else if ( key == "volume" )
			{
				if ( auto n = to_number( value ) )
				{
					bgm.volume = * n;
				}
				else
				{
					warn_type( value, "bgm.volume", "数" );
				}
			}
			else
			{
				warn( k.source(), "bgm の知らないキー " + key + " を飛ばします" );
			}
		}

		data_.bgm = bgm;
	}
};

} // namespace

StoryTextData StoryTextParser::parse_text( const string_t& text, const string_t& file_name )
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

	StoryTextData data;
	Reader reader( file_name, data );

	// 形式の版（読めない版は出せない）
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

	for ( const auto& [ k, v ] : root )
	{
		const string_t key( k.str() );

		if ( key == "format" || key == "layer" )
		{
			continue;
		}
		else if ( key == "sound" )
		{
			data.sound = reader.read_string( v, key );
		}
		else if ( key == "next_scene" )
		{
			data.next_scene = reader.read_string( v, key );
		}
		else if ( key == "next_stage" )
		{
			data.next_stage = reader.read_string( v, key );
		}
		else if ( key == "bgm" )
		{
			reader.read_bgm( v );
		}
		else if ( key == "text" )
		{
			if ( const toml::table* t = v.as_table() )
			{
				reader.read_text( * t );
			}
			else
			{
				reader.warn( v.source(), "[text] は表で書きます。飛ばします" );
			}
		}
		else
		{
			reader.warn( k.source(), "知らないキー " + key + " を飛ばします" );
		}
	}

	// 重なりは配列の順に描く（表のキーの順は TOML では意味を持たないので、順番は配列で決める）
	if ( const toml::node* node = root.get( "layer" ) )
	{
		if ( const toml::array* a = node->as_array() )
		{
			for ( const auto& v : * a )
			{
				reader.read_layer( v );
			}
		}
		else
		{
			reader.warn( node->source(), "[[layer]] は表の配列で書きます。飛ばします" );
		}
	}

	return data;
}

StoryTextData StoryTextParser::parse_file( const string_t& file_name )
{
	std::ifstream in( file_name, std::ios::binary );

	if ( ! in.good() )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( "ステージの前後の文のファイル \"" + file_name + "\" を開けません" );
	}

	std::stringstream ss;
	ss << in.rdbuf();

	return parse_text( ss.str(), file_name );
}

} // namespace blue_sky::stage
