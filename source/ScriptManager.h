#pragma once

#include <common/math.h>
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

#include <queue>

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

	std::deque< std::string > command_history_;
	int command_history_index_;

	const std::string empty_command_;

public:
	ScriptManager();
	~ScriptManager();

	/*
	template< typename ResultType, typename... ArgTypes >
	void set_function( const string_t& name, const std::function< ResultType ( ArgTypes... ) >& function )
	{
		lua_.set_function( name, [ function ] ( ArgTypes... args... ) -> ResultType
		{
			try
			{
				return function( args... );
			}
			catch ( ... )
			{

			}
		} );
	}
	*/

	/**
	 * �X�N���v�g����Ăяo����֐���ݒ肷��
	 *
	 * @param name �X�N���v�g����Ăяo�����̊֐���
	 * @param function �֐�
	 * @todo �֐�����O�𓊂������ɃL���b�`�ł���悤�ɂ���
	 */
	template< typename FunctionType >
	void set_function( const string_t& name, const FunctionType& function )
	{
		lua_.set_function( name, function);
	}

	void exec( const string_t& );

	template< typename Type >
	Type get( const string_t& name ) const
	{
		return lua_.get< Type >( name );
	}

	const std::string& get_current_history_command() const { return ( command_history_index_ < static_cast< int >( command_history_.size() ) ? command_history_[ command_history_index_ ] : empty_command_ ); }
	const std::string& get_prev_hisotry_command() { command_history_index_ = math::clamp< int >( command_history_index_ - 1, 0, command_history_.size() ); return get_current_history_command(); }
	const std::string& get_next_hisotry_command() { command_history_index_ = math::clamp< int >( command_history_index_ + 1, 0, command_history_.size() ); return get_current_history_command(); }
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
	command_history_.push_back( script );
	command_history_index_ = command_history_.size();

	if ( command_history_.size() > 100 )
	{
		 command_history_.pop_front();
	}

	auto result = lua_.safe_script( script );

	/*
	sol::protected_function_result result;

	try
	{
		result = lua_.safe_script( script );
	}
	catch ( ... )
	{
		ScriptError e( "unknown script error." );
		throw e;
	}
	*/

	if ( ! result.valid() )
	{
		ScriptError e = result;
		throw e;
	}
}

}