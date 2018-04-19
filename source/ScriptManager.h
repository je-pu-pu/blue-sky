#pragma once

#include <common/exception.h>
#include <type/type.h>

/**
 * Sol 2.20.0 �ł� set_function() �Ŋ֐��ǉ���A��`����Ă��Ȃ��ϐ����g���Ċ֐����Ăяo���ƁA
 * �G���[����������Ɠ����� 8 �o�C�g�̃��������[�N����������B
 *
 * @todo ���������[�N�𒲍����� or Sol �̃o�[�W������ς���
 */
#define SOL_NO_EXCEPTIONS 1
#include <sol/sol.hpp>

namespace blue_sky
{

typedef sol::error ScriptError;

/**
 * �X�N���v�g�Ǘ�
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