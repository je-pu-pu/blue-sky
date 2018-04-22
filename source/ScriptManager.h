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

#include <boost/algorithm/string.hpp>

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

	string_t output_;

	std::deque< string_t > command_history_;
	int command_history_index_;

	const string_t empty_command_;

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

	void auto_complete( string_t& );

	void exec( const string_t& );

	template< typename Type >
	Type get( const string_t& name ) const
	{
		return lua_.get< Type >( name );
	}

	const string_t& get_current_history_command() const { return ( command_history_index_ < static_cast< int >( command_history_.size() ) ? command_history_[ command_history_index_ ] : empty_command_ ); }
	const string_t& get_prev_hisotry_command() { command_history_index_ = math::clamp< int >( command_history_index_ - 1, 0, command_history_.size() ); return get_current_history_command(); }
	const string_t& get_next_hisotry_command() { command_history_index_ = math::clamp< int >( command_history_index_ + 1, 0, command_history_.size() ); return get_current_history_command(); }

	const string_t& get_output() const { return output_; }
};

inline ScriptManager::ScriptManager()
{
	// lua_.open_libraries( sol::lib::base, sol::lib::package );
}

inline ScriptManager::~ScriptManager()
{
	
}

inline void ScriptManager::auto_complete( string_t& script )
{
	output_.clear();

	auto last_space = script.find_last_of( " ,=()" );
	
	string_t::size_type input_word_pos = 0;
	string_t input_word;

	if ( last_space != string_t::npos )
	{
		input_word_pos = last_space + 1;
		input_word = script.substr( input_word_pos );
	}
	else
	{
		input_word_pos = 0;
		input_word = script;
	}

	std::vector< string_t > candidate_list;

	for ( const auto& v : lua_ )
	{
		string_t identifier = v.first.as< string_t >();

		if ( boost::starts_with( identifier, input_word ) )
		{
			candidate_list.push_back( std::move( identifier ) );
		}
	}

	if ( candidate_list.empty() )
	{
		return;
	}

	if ( candidate_list.size() == 1 )
	{
		script = script.substr( 0, input_word_pos ) + candidate_list.front();
		return;
	}

	for ( const auto& candidate : candidate_list )
	{
		output_ += candidate + "\n";
	}

	output_.resize( output_.size() - 1 );
}

inline void ScriptManager::exec( const string_t& script )
{
	output_.clear();

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