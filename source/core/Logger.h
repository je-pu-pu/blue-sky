#pragma once

#include "type.h"
#include <deque>

#undef ERROR

namespace core
{

/**
 * ログ管理クラス
 *
 */
class Logger
{
public:
	class Log
	{
	public:
		enum class Type
		{
			ERROR,
			WARN,
			INFO,
			DEBUG
		};

	private:
		Type type_;
		string_t message_;

	public:
		Log( Type t, const string_t& m )
			: type_( t )
			, message_( m )
		{ }

		Type get_type() const { return type_; }
		const string_t& get_message() const { return message_; }
	};

private:
	typedef std::deque< Log > LogList;
	LogList log_list_;

public:
	void error( const string_t& m )
	{
		log_list_.emplace_back( Log::Type::ERROR, m );
	}

	void warn( const string_t& m )
	{
		log_list_.emplace_back( Log::Type::WARN, m );
	}

	void info( const string_t& m )
	{
		log_list_.emplace_back( Log::Type::INFO, m );
	}

	void debug( const string_t& m )
	{
		log_list_.emplace_back( Log::Type::DEBUG, m );
	}

	const LogList& get_log_list() const { return log_list_; }
};

Logger logger;

} // namespace core
