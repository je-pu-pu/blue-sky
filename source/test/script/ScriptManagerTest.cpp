#include "pch.h"

#include <blue_sky/ScriptManager.h>

#include <stdexcept>

// スクリプトから呼んだ C++ の関数が例外を投げても、ゲームが落ちずに ScriptError として受け取れること
TEST( ScriptManagerTest, 関数が投げた標準の例外は_exec_が_ScriptError_として投げる )
{
	blue_sky::ScriptManager script_manager;
	script_manager.set_function( "fail", [] () -> int { throw std::runtime_error( "失敗した" ); } );

	try
	{
		script_manager.exec( "fail()" );
		FAIL() << "例外が投げられなかった";
	}
	catch ( const blue_sky::ScriptError& e )
	{
		EXPECT_NE( std::string( e.what() ).find( "失敗した" ), std::string::npos ) << e.what();
	}
}

TEST( ScriptManagerTest, 標準の例外でないものを投げても_ScriptError_になる )
{
	blue_sky::ScriptManager script_manager;
	script_manager.set_function( "fail", [] () -> int { throw 1; } );

	EXPECT_THROW( script_manager.exec( "fail()" ), blue_sky::ScriptError );
}

TEST( ScriptManagerTest, 例外の後もスクリプトを続けて実行できる )
{
	blue_sky::ScriptManager script_manager;
	script_manager.set_function( "fail", [] () -> int { throw std::runtime_error( "失敗した" ); } );

	EXPECT_THROW( script_manager.exec( "fail()" ), blue_sky::ScriptError );

	script_manager.exec( "x = 1 + 2" );
	EXPECT_EQ( 3, script_manager.get< int >( "x" ) );
}
