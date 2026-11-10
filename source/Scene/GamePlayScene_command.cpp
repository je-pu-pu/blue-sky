#include "GamePlayScene.h"
#include "StageSelectScene.h"

#include <blue_sky/GameMain.h>

#include <GameObject/Player.h>
#include <GameObject/Camera.h>

#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/graphics/Model.h>

#include <core/animation/AnimationPlayer.h>
#include <core/sound/SoundManager.h>

#include <blue_sky/ActiveObjectPhysics.h>
#include <blue_sky/ActiveObjectManager.h>
#include <blue_sky/ObjectFactory.h>
#include <blue_sky/ScriptManager.h>

#include <blue_sky/DelayedCommand.h>
#include <blue_sky/stage/TomlStageParser.h>
#include <blue_sky/stage/StageBuilder.h>

#include <core/sound/Sound.h>
#include <game/Config.h>

#include <common/exception.h>
#include <common/serialize.h>
#include <common/math.h>

#include <fstream>
#include <sstream>

namespace blue_sky
{

/**
 * プレイヤーの前方にオブジェクトを生成する（スクリプトの create_object_at_player_front）
 *
 * @param class_name 種類の名前
 * @return 生成したオブジェクト（知らない種類なら std::nullopt。スクリプトには nil）
 */
std::optional< GameObjectRef > GamePlayScene::create_object_at_player_front( const char_t* class_name )
{
	auto id = get_object_factory()->create_object( class_name );

	if ( ! id )
	{
		return std::nullopt;
	}

	GameObjectRef o = get_object_factory()->get_ref( * id );

	o.set_start_location( player_->get_location().x(), player_->get_location().y(), player_->get_location().z() + 3.f );
	o.set_start_direction_degree( 0.f );

	o.restart();

	return o;
}

/**
 * スクリプトから呼べる関数を準備する
 *
 * ステージの動き（以前の command_map_ の 36 個のコマンド）は、読み込むときに stage::Action にし、run_action で行う
 */
void GamePlayScene::setup_command()
{
	// --- スクリプト ---
	get_script_manager()->set_function( "create_object_at_player_front", [this] ( const char_t* class_name ) -> sol::optional< GameObjectRef >
	{
		if ( auto o = create_object_at_player_front( class_name ) )
		{
			return * o;
		}

		return sol::nullopt;
	} );
}

/**
 * ステージの動きを 1 つ行う（以前は command_map_ の名前で引き、文字列の引数を解釈していた）
 *
 * 時間を置く動き（delay・every。以前の timer）は、時間を置く一覧（delayed_command_list_）に預ける。
 * 書いていない引数は以前と同じ既定の値を使う（色は書いた成分だけ変える・位置の足りない数は 0 など。読む側で決めてある）
 */
void GamePlayScene::run_action( const stage::Action& a )
{
	using stage::ActionType;

	if ( a.is_timed )
	{
		stage::Action now = a;
		now.is_timed = false;

		delayed_command_list_.push_back( std::make_unique< DelayedCommand >( static_cast< float_t >( a.interval ), a.count, now ) );
		return;
	}

	auto to_vector = [] ( const std::vector< double >* v )
	{
		return v ? Vector( static_cast< float_t >( ( * v )[ 0 ] ), static_cast< float_t >( ( * v )[ 1 ] ), static_cast< float_t >( ( * v )[ 2 ] ) ) : Vector::Zero;
	};

	// 色は書いた成分だけ変える（以前の ss >> r >> g >> b >> a は、読めなかった所から後を前の値のままにした）
	auto set_components = [] ( Color& color, const std::vector< double >& v )
	{
		float_t* c[] = { & color.r(), & color.g(), & color.b(), & color.a() };

		for ( size_t i = 0; i < v.size() && i < 4; ++i )
		{
			* c[ i ] = static_cast< float_t >( v[ i ] );
		}
	};

	auto number = [ & ] ( const char* key, double default_value ) { return static_cast< float_t >( a.number( key, default_value ) ); };

	// 名前で引いた物体（見つからなければ何もしない。以前と同じ）
	std::optional< GameObjectRef > target = a.has( "target" ) ? get_object_factory()->find_ref( a.text( "target" ) ) : std::nullopt;

	switch ( a.type )
	{
		case ActionType::SET:
		{
			if ( a.has( "sky_box" ) )
			{
				if ( a.text( "sky_box" ) == "none" )
				{
					get_graphics_manager()->unset_sky_box();
				}
				else
				{
					get_graphics_manager()->set_sky_box( ( "sky-box-" + a.text( "sky_box" ) ).c_str() );
				}
			}

			if ( a.has( "bgm" ) )
			{
				bgm_ = get_sound_manager()->load_music( "bgm", a.text( "bgm" ).c_str(), true );
				bgm_->play( bgm_loop_ );
			}

			if ( a.has( "bpm" ) )
			{
				set_bpm( number( "bpm", 120.0 ) * number( "beat", 1.0 ) );
			}

			if ( a.has( "line_type" ) ) { get_graphics_manager()->set_drawing_line_type( static_cast< int >( a.number( "line_type", 0.0 ) ) ); }
			if ( a.has( "paper_type" ) ) { get_graphics_manager()->set_paper_texture_type( static_cast< int >( a.number( "paper_type", 0.0 ) ) ); }
			if ( a.has( "drawing_accent_scale" ) ) { drawing_accent_scale_ = number( "drawing_accent_scale", 0.0 ); }
			if ( a.has( "shading_enabled" ) ) { shading_enabled_ = a.flag( "shading_enabled", true ); }
			if ( a.has( "balloon_sound_type" ) ) { balloon_sound_type_ = static_cast< BalloonSoundType >( static_cast< int_t >( a.number( "balloon_sound_type", 0.0 ) ) ); }

			if ( auto v = a.numbers( "ambient_color" ) ) { set_components( ambient_color_.target_value(), * v ); ambient_color_.chase_full(); }
			if ( auto v = a.numbers( "shadow_color" ) ) { set_components( shadow_color_.target_value(), * v ); shadow_color_.chase_full(); }
			if ( auto v = a.numbers( "shadow_paper_color" ) ) { set_components( shadow_paper_color_.target_value(), * v ); shadow_paper_color_.chase_full(); }

			if ( auto v = a.numbers( "light_position" ) )
			{
				const Vector p = to_vector( v );
				light_position_.target_value().set( p.x(), p.y(), p.z() );
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
						color.speed() = number( "speed", 0.0 );
					}
				}
			};

			fade( "ambient_color", ambient_color_ );
			fade( "shadow_color", shadow_color_ );
			fade( "shadow_paper_color", shadow_paper_color_ );

			if ( auto v = a.numbers( "light_position" ) )
			{
				const Vector p = to_vector( v );
				light_position_.target_value().set( p.x(), p.y(), p.z() );
			}

			break;
		}
		case ActionType::PLAY_SOUND:
		{
			if ( Sound* sound = get_sound_manager()->load( a.text( "sound" ).c_str() ) )
			{
				sound->play( a.flag( "loop", false ), a.flag( "force", true ) );
			}

			break;
		}
		case ActionType::STOP_SOUND:
		{
			if ( Sound* sound = get_sound_manager()->load( a.text( "sound" ).c_str() ) )
			{
				sound->stop();
			}

			break;
		}
		case ActionType::STOP_ALL_SOUNDS:
		{
			get_sound_manager()->stop_all();
			break;
		}
		case ActionType::LOAD_SOUND:
		{
			const string_t& sound_name = a.text( "name" );
			const string_t& file = a.has( "file" ) ? a.text( "file" ) : sound_name;

			// replace なら読み込み済みでも読み直す（番号を変えずに中身を差し替える）
			if ( a.flag( "replace", false ) )
			{
				get_sound_manager()->reload( sound_name.c_str(), file.c_str() );
			}
			else
			{
				get_sound_manager()->load( sound_name.c_str(), file.c_str() );
			}

			break;
		}
		case ActionType::SPAWN:
		{
			get_object_factory()->create_static_object( a.text( "model" ), to_vector( a.numbers( "position" ) ), to_vector( a.numbers( "rotation" ) ) );
			break;
		}
		case ActionType::MOVE_TO:
		{
			if ( target ) { target->set_target_location( to_vector( a.numbers( "position" ) ), number( "speed", 0.0 ) ); }
			break;
		}
		case ActionType::TURN_TO:
		{
			if ( ! target )
			{
				break;
			}

			if ( a.has( "face" ) )
			{
				if ( auto to = get_object_factory()->find_ref( a.text( "face" ) ) )
				{
					target->set_target_direction_to( * to, number( "speed", 0.0 ) );
				}
			}
			else
			{
				target->set_target_direction( number( "direction", 0.0 ), number( "speed", 0.0 ) );
			}

			break;
		}
		case ActionType::SET_BODY:
		{
			if ( ! target )
			{
				break;
			}

			if ( a.has( "mass" ) ) { target->set_mass( number( "mass", 0.0 ) ); }
			if ( auto v = a.numbers( "gravity" ) ) { target->set_gravity( to_vector( v ) ); }
			if ( auto v = a.numbers( "angular_factor" ) ) { target->set_angular_factor( to_vector( v ) ); }
			if ( a.has( "kinematic" ) ) { target->set_kinematic( a.flag( "kinematic", false ) ); }
			if ( a.has( "contact_response" ) ) { target->set_no_contact_response( ! a.flag( "contact_response", true ) ); }
			break;
		}
		case ActionType::SET_FLICKER:
		{
			if ( target ) { target->set_flicker_scale( number( "scale", 1.0 ) ); }
			break;
		}
		case ActionType::START_FLICKERING:
		{
			player_->start_flickering();
			break;
		}
		case ActionType::PLAY_ANIMATION:
		{
			if ( target ) { target->play_animation( a.text( "name" ).c_str(), a.flag( "force", true ), a.flag( "loop", false ) ); }
			break;
		}
		case ActionType::SET_ANIMATION_SPEED:
		{
			if ( target ) { target->set_animation_speed( number( "speed", 1.0 ) ); }
			break;
		}
		case ActionType::SWITCH:
		{
			// イベントは起こさない（以前の game_object.action の turn-on・turn-off・break）
			const string_t& state = a.text( "state" );

			if ( target ) { target->action( state == "on" ? "turn-on" : state == "off" ? "turn-off" : "break" ); }
			break;
		}
		case ActionType::SET_MODE:
		{
			if ( target ) { target->action( a.text( "mode" ) ); }
			break;
		}
		case ActionType::SET_VISIBLE:
		{
			if ( target ) { target->action( string_t( "set_visible " ) + ( a.flag( "model", true ) ? "1" : "0" ) + " " + ( a.flag( "line", true ) ? "1" : "0" ) ); }
			break;
		}
		case ActionType::BREAK_ANIMATION:
		{
			if ( target ) { target->action( a.flag( "broken", true ) ? "break_animation 1" : "break_animation 0" ); }
			break;
		}
		case ActionType::END_STAGE:
		{
			if ( bgm_ ) { bgm_->stop(); }
			get_sound_manager()->stop_all();

			if ( Sound* sound = get_sound_manager()->get_sound( "switch-off" ) )
			{
				sound->play( false, true );
			}

			is_blackout_ = true;
			break;
		}
	}
}

void GamePlayScene::save_stage_file( const char* ) const
{

}

/**
 * ステージファイルを読み込む
 *
 * @param base_name ステージのファイル名から .stage.toml を除いたもの（"media/stage/2-1" など）
 */
void GamePlayScene::load_stage_file( const char* base_name )
{
	// 読む（ファイル → StageData）。以前の独自の形式（.stage）は 2026-11-10 に TOML へ移して消した（変換ツールは tools/stage_convert）
	const stage::StageData data = stage::TomlStageParser::parse_file( string_t( base_name ) + ".stage.toml" );

	// 当てる（StageData → ゲーム）
	stage::StageBuilder::Context context;
	context.object_factory = get_object_factory();
	context.physics = get_active_object_physics();
	context.graphics_manager = get_graphics_manager();
	context.sound_manager = get_sound_manager();
	context.player = player_;
	context.camera = camera_.get();
	context.goal = goal_;
	context.stage_dir = StageSelectScene::get_stage_dir_name_by_page( get_save_data()->get( "stage-select.page", 0 ) );
	context.set_bpm = [this] ( float_t bpm ) { set_bpm( bpm ); };
	context.make_call = [this] ( const stage::Action& action ) -> stage::StageBuilder::Call
	{
		return [ this, action ] { run_action( action ); };
	};

	const auto result = stage::StageBuilder::build( data, context );

	stage_setup_command_call_list_.assign( result.start_calls.begin(), result.start_calls.end() );
	bgm_loop_ = result.bgm_loop;
}

} // namespace blue_sky
