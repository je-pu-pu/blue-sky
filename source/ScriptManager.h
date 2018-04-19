#pragma once

#include <common/exception.h>
#include <type/type.h>

/**
 * Sol 2.20.0 では set_function() で関数追加後、定義されていない変数を使って関数を呼び出すと、
 * エラーが発生すると同時に 8 バイトのメモリリークが発生する。
 *
 * @todo メモリリークを調査する or Sol のバージョンを変える
 */
#define SOL_NO_EXCEPTIONS 1
#include <sol/sol.hpp>

namespace blue_sky
{

typedef sol::error ScriptError;

/**
 * スクリプト管理
 */
class ScriptManager
{
private:
	sol::state lua_;

public:
	ScriptManager();
	~ScriptManager();

	template< typename FunctionType >
	void set_function( const string_t& name, const FunctionType& function )
	{
		lua_.set_function( name, function );
	}

	void exec( const string_t& );

	template< typename Type >
	Type get( const string_t& name ) const
	{
		return lua_.get< Type >( name );
	}
};

inline ScriptManager::ScriptManager()
{
	// lua_.open_libraries( sol::lib::base, sol::lib::package );
}

inline ScriptManager::~ScriptManager()
{
	
}

inline void ScriptManager::exec( const string_t& script )
{
	auto result = lua_.safe_script( script );

	if ( ! result.valid() )
	{
		ScriptError e = result;
		throw e;
	}
}

}