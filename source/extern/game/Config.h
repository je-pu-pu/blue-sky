#pragma once

#include <common/serialize.h>
#include <functional>
#include <map>
#include <set>
#include <string>

namespace game
{

class Config
{
public:
	using ValueMap = std::map< std::string, std::string >;

	/// 読めない行・値を知らせる先（この層はアプリのログを知らないので、組み立ての根が debug_log などを渡す）
	using LogHandler = std::function< void( const std::string& ) >;

private:
	ValueMap value_;
	std::map< std::string, std::string > origin_;		///< 名前 → どこで読んだか（"ファイル名:行番号"）。load_file で読んだ値だけ
	LogHandler log_;
	mutable std::set< std::string > reported_;		///< 読めない値を知らせ済みの名前（毎フレーム読まれても 1 回だけ出す）

	void read_line( const std::string& line, const std::string& origin );
	void report_unreadable_value( const char* name, const std::string& value ) const;

public:
	Config();
	virtual ~Config();

	/// 読めない行・値を知らせる先を決める（決めなければ知らせない）
	void set_log_handler( LogHandler handler ) { log_ = std::move( handler ); }

	/// その名前の値をどこで読んだか（"ファイル名:行番号"）。load_file で読んでいなければ空
	std::string get_origin( const char* name ) const;

	template< typename T > T get( const char* name, T default_value ) const
	{
		ValueMap::const_iterator i = value_.find( name );

		if ( i == value_.end() )
		{
			return default_value;
		}

		try
		{
			return common::deserialize< T >( i->second );
		}
		catch ( const boost::bad_lexical_cast& )
		{
			// 黙って既定値にせず、どこの値が読めなかったかを知らせる
			report_unreadable_value( name, i->second );
			return default_value;
		}
	}

	// 以前は const でない get が「無い・読めない」ときに既定値を書き込んでいた。
	// その副作用で、一度起動するだけで全既定値がファイルに固定され、コード側の既定値の変更が届かなかった。
	// 読むだけにし（上の const 版だけを残す）、書き込みは set() で明示する。

	template< typename T > void set( const char* name, T value ) { value_[ name ] = common::serialize( value ); origin_.erase( name ); reported_.erase( name ); }

	/// 項目を消す（無ければ何もしない）
	void erase( const char* name ) { value_.erase( name ); origin_.erase( name ); reported_.erase( name ); }

	bool has( const char* name ) const { return value_.find( name ) != value_.end(); }

	const ValueMap& get_values() const { return value_; }

	bool save_file( const char* );
	bool load_file( const char* );

	void read_line( const std::string& );

}; // class Config

} // namespace game
