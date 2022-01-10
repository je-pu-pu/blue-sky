#pragma once

#include <GameObject/ActiveObject.h>
#include <blue_sky/graphics/Model.h>
#include <blue_sky/graphics/Mesh.h>

#include <core/graphics/Shader.h>

#include <common/math.h>
#include <common/exception.h>
#include <type/type.h>

/**
 * Sol 2.20.0 �ł� set_function() �Ŋ֐��ǉ���A��`����Ă��Ȃ��ϐ����g���Ċ֐����Ăяo���ƁA
 * �G���[����������Ɠ����� 8 �o�C�g�̃��������[�N����������B
 *
 * @todo ���������[�N�𒲍����� or Sol �̃o�[�W������ς���
 */
// #define SOL_NO_EXCEPTIONS 1
#include <sol/sol.hpp>

#include <boost/algorithm/string.hpp>

#include <queue>
#include <fstream>

namespace blue_sky
{

using ScriptError = sol::error;

/**
 * �X�N���v�g�Ǘ�
 *
 * game �� blue_sky �ɕ�������
 */
class ScriptManager // : game::ScriptManager
{
public:
	using Model		= graphics::Model;
	using Mesh		= graphics::Mesh;
	using Shader	= core::graphics::Shader;
	using Texture	= core::graphics::Texture;

	const size_t MAX_COMMAND_HISTORY_SIZE = 1000; ///< �R�}���h�����̍ő吔

private:
	sol::state lua_;

	string_t output_;

	std::deque< string_t > command_history_;
	int command_history_index_ = 0;

	const string_t empty_command_;

protected:
	void setup_user_types()
	{
		lua_.new_usertype< ActiveObject >(
			"GameObject",
			"model", sol::property( sol::resolve< Model* () >( & ActiveObject::get_model ), & ActiveObject::set_model ),
			"play_animation", & ActiveObject::play_animation,

			"kill", & ActiveObject::kill
        );

		lua_.new_usertype< Model >(
			"Model",
			"mesh", sol::property( sol::resolve< Mesh* () >( & Model::get_mesh ), & Model::set_mesh ),
			"get_shader_at", sol::resolve< Shader* ( uint_t ) >( & Model::get_shader_at ),
			"set_shader_at", & graphics::Model::set_shader_at
		);

		lua_.new_usertype< Shader >(
			"Shader",
			"get_texture_at", sol::resolve< Shader::Texture* ( uint_t ) >( & Shader::get_texture_at ),
			"set_texture_at", & Shader::set_texture_at
		);

		// @todo Texture::load() ���������ăX�N���v�g����e�N�X�`���̓��e�X�V���ł���悤�ɂ���H
		/*
		lua_.new_usertype< Texture >(
			"Texture",
			"load", & Texture::load
		);
		*/
	}

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
		lua_.set_function( name, function );
	}

	void auto_complete( string_t& );

	void exec( const string_t&, bool = false );

	template< typename Type >
	Type get( const string_t& name ) const
	{
		return lua_.get< Type >( name );
	}

	const string_t& get_current_history_command() const { return ( command_history_index_ < static_cast< int >( command_history_.size() ) ? command_history_[ command_history_index_ ] : empty_command_ ); }
	const string_t& get_prev_hisotry_command() { command_history_index_ = math::clamp< int >( command_history_index_ - 1, 0, command_history_.size() ); return get_current_history_command(); }
	const string_t& get_next_hisotry_command() { command_history_index_ = math::clamp< int >( command_history_index_ + 1, 0, command_history_.size() ); return get_current_history_command(); }

	const std::deque< string_t >& get_command_history() const { return command_history_; }

	void load_command_history( const string_t& );
	void save_command_history( const string_t& );

	void truncate_command_history( uint_t );

	const string_t& get_output() const { return output_; }
};

inline ScriptManager::ScriptManager()
{
	lua_.open_libraries( sol::lib::base, sol::lib::package );

	setup_user_types();
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

inline void ScriptManager::exec( const string_t& script, bool add_history )
{
	output_.clear();

	if ( add_history )
	{
		command_history_.push_back( script );
		command_history_index_ = command_history_.size();
	}

	if ( command_history_.size() > MAX_COMMAND_HISTORY_SIZE )
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

/**
 * �R�}���h�������t�@�C������ǂݍ���
 *
 * @param file_path �ǂݍ��ރt�@�C���̃p�X
 */
inline void ScriptManager::load_command_history( const string_t& file_path )
{
	std::ifstream in( file_path );

	while( in && ! in.eof() )
	{
		string_t command;
		
		in >> command;
		command_history_.push_back( command );
	}

	command_history_index_ = command_history_.size() - 1;

	truncate_command_history( MAX_COMMAND_HISTORY_SIZE );
}

/**
 * �R�}���h�������t�@�C���ɏ����o��
 *
 * @param file_path �����o���t�@�C���̃p�X
 */
inline void ScriptManager::save_command_history( const string_t& file_path )
{
	std::ofstream of( file_path );

	for ( const auto& c : command_history_ )
	{
		of << c << '\n';
	}
}

/**
 * �R�}���h�����̒�����؂�l�߂�
 *
 * @param size ����
 */
inline void ScriptManager::truncate_command_history( uint_t size )
{
	if ( command_history_.size() <= size )
	{
		return;
	}

	command_history_.erase( command_history_.begin(), command_history_.begin() + ( command_history_.size() - size ) );
	
	command_history_index_ = command_history_.size() - 1;
}

}