#include "pch.h"

#include <blue_sky/ScriptBindings.h>
#include <blue_sky/ScriptManager.h>

#include <vector>

using blue_sky::ScriptBindings;
using blue_sky::ScriptManager;
using blue_sky::SceneRequest;

namespace
{

/// シーンの切り替えの要求だけを受け取って登録する（ほかの窓口は呼ばれないので nullptr）
class ScriptBindingsTest : public ::testing::Test
{
protected:
	ScriptManager script;
	std::vector< SceneRequest > requests;

	void SetUp() override
	{
		ScriptBindings::Targets targets;
		targets.request_scene = [this] ( const SceneRequest& r ) { requests.push_back( r ); };

		ScriptBindings::bind( script, targets );
	}
};

} // namespace

/// scene・push_overlay・pop_overlay はその場で切り替えず、要求を出す（名前の後の "?キー=値" は引数）
TEST_F( ScriptBindingsTest, SceneFunctionsRequestTransitions )
{
	script.exec( "scene( 'stage_select' )" );
	script.exec( "push_overlay( 'options?tab=controls' )" );
	script.exec( "pop_overlay()" );

	ASSERT_EQ( requests.size(), 3u );
	EXPECT_EQ( requests[ 0 ], ( SceneRequest { SceneRequest::Type::REPLACE_BASE, "stage_select" } ) );
	EXPECT_EQ( requests[ 1 ], ( SceneRequest { SceneRequest::Type::PUSH_OVERLAY, "options", "", { { "tab", "controls" } } } ) );
	EXPECT_EQ( requests[ 2 ], ( SceneRequest { SceneRequest::Type::POP_OVERLAY } ) );
}

/// 窓口の要らない関数（color・vector）と、nil を渡すとスクリプトのエラーになる関数
TEST_F( ScriptBindingsTest, BasicFunctionsAndNilArguments )
{
	script.exec( "c = type( color( 0.25, 0.5, 0.75, 1 ) )" );
	script.exec( "v = type( vector( 1, 2, 3 ) )" );
	EXPECT_EQ( script.get< std::string >( "c" ), "userdata" );
	EXPECT_EQ( script.get< std::string >( "v" ), "userdata" );

	EXPECT_THROW( script.exec( "set_loc( nil, 1, 2, 3 )" ), blue_sky::ScriptError );
	EXPECT_THROW( script.exec( "clone_object( nil )" ), blue_sky::ScriptError );
}

/// load（media/script/ の下のファイルを読む）が定義されている
TEST_F( ScriptBindingsTest, LoadIsDefined )
{
	script.exec( "t = type( load )" );
	EXPECT_EQ( script.get< std::string >( "t" ), "function" );
}
