#pragma once

#include <bitset>
#include <limits>

namespace common
{

/**
 * 列挙型 ( enum ) の集合を保持する
 */
template< typename EnumType, size_t N >
class enum_set
{
public:

private:
	std::bitset< N > set_;

public:
	/**
	 * どの要素も持たない空の集合を作成する
	 */
	enum_set() { }

	/**
	 * 指定された要素のみを持つ集合を作成する
	 */
	enum_set( EnumType e )
		: set_( 1 << static_cast< int >( e ) )
	{ }

	/**
	 * リストで指定された要素を持つ集合を作成する
	 */
	enum_set( std::initializer_list< EnumType > es )
	{
		for ( auto e : es )
		{
			set_ |= 1 << static_cast< int >( e );
		}
	}

	/**
	 * 引数で指定された要素を持っているかどうかを返す
	 */
	bool test( EnumType e ) const { return set_.test( static_cast< size_t >( e ) ); }

	// friend constexpr enum_set operator | ( enum_set s, EnumType e ) { s.set_ |= 1 << static_cast< int >( e ); return s; }
	// friend constexpr enum_set operator | ( EnumType e1, EnumType e2 ) { enum_set s( e1 ); return s | e2; }
};

} // namespace common

/*
ShaderStage::VS | ShaderStage::PS としても、この関数は何故か呼ばれない
template< typename EnumType >
common::enum_set< EnumType, EnumType::Max > operator | ( EnumType e1, EnumType e2 )
{
    common::enum_set< EnumType, EnumType::Max > s;
	s( e1 );
	s |= e2;

	return s;
}
*/

/*
// ビット数を 32 に固定すれば operator | を使えるが、美しくないので却下
template< typename EnumType >
common::enum_set< EnumType, 32 > operator | ( EnumType e1, EnumType e2 ) { common::enum_set< EnumType, 32 > s( e1 ); return s | e2; }
*/

/*
// s.set_ は private なのでコンパイルエラー
template< typename EnumType >
constexpr common::enum_set< EnumType, 32 > operator | ( const common::enum_set< EnumType, 32 > s, EnumType e ) { s.set_ |= e; return s; }

template< typename EnumType >
constexpr common::enum_set< EnumType, 32 > operator | ( EnumType e1, EnumType e2 ) { common::enum_set< EnumType, 32 > s( e1 ); return s | e2; }
*/