#include "LegacyStageParser.h"

#include <common/exception.h>

#include <istream>
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

			Action action;
			ss >> action.name;
			action.params = join_params( ss );
			action.where = where();

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

	return data;
}

} // namespace blue_sky::stage
