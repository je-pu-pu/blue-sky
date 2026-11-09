#pragma once

#include <common/serialize.h>
#include <map>

namespace game
{

class Config
{
public:
	using ValueMap = std::map< std::string, std::string >;

private:
	ValueMap value_;

public:
	Config();
	virtual ~Config();

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
		catch ( boost::bad_lexical_cast )
		{
			return default_value;
		}
	}

	// 以前は const でない get が「無い・読めない」ときに既定値を書き込んでいた。
	// その副作用で、一度起動するだけで全既定値がファイルに固定され、コード側の既定値の変更が届かなかった。
	// 読むだけにし（上の const 版だけを残す）、書き込みは set() で明示する。

	template< typename T > void set( const char* name, T value ) { value_[ name ] = common::serialize( value ); }

	/// 項目を消す（無ければ何もしない）
	void erase( const char* name ) { value_.erase( name ); }

	bool has( const char* name ) const { return value_.find( name ) != value_.end(); }

	const ValueMap& get_values() const { return value_; }

	bool save_file( const char* );
	bool load_file( const char* );

	void read_line( const std::string& );

}; // class Config

} // namespace game
