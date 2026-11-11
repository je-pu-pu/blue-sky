#include "pch.h"

#include <blue_sky/KeyNames.h>
#include <blue_sky/Input.h>

#include <windows.h>

namespace key_names = blue_sky::key_names;

TEST( KeyNamesTest, 英数字1文字は従来の設定ファイルと互換 )
{
	EXPECT_EQ( 'A', key_names::code_from_name( "A" ) );
	EXPECT_EQ( 'W', key_names::code_from_name( "w" ) );
	EXPECT_EQ( '1', key_names::code_from_name( "1" ) );
	EXPECT_EQ( "A", key_names::name_from_code( 'A' ) );
}

TEST( KeyNamesTest, 名前付きキーは大文字小文字を区別しない )
{
	EXPECT_EQ( VK_LBUTTON, key_names::code_from_name( "MOUSE_L" ) );
	EXPECT_EQ( VK_SPACE, key_names::code_from_name( "space" ) );
	EXPECT_EQ( VK_LSHIFT, key_names::code_from_name( "LShift" ) );
	EXPECT_EQ( VK_F12, key_names::code_from_name( "F12" ) );
}

TEST( KeyNamesTest, 別名は先に定義した名前に正規化される )
{
	EXPECT_EQ( VK_RETURN, key_names::code_from_name( "RETURN" ) );
	EXPECT_EQ( "ENTER", key_names::name_from_code( VK_RETURN ) );
	EXPECT_EQ( "CTRL", key_names::name_from_code( VK_CONTROL ) );
}

TEST( KeyNamesTest, 不明な名前とコード )
{
	EXPECT_EQ( 0, key_names::code_from_name( "" ) );
	EXPECT_EQ( 0, key_names::code_from_name( "NO_SUCH_KEY" ) );
	EXPECT_EQ( 0, key_names::code_from_name( "!" ) );
	EXPECT_EQ( "", key_names::name_from_code( 0 ) );
}

TEST( KeyNamesTest, 全ての割り当て可能キーは名前と往復できる )
{
	for ( int code : key_names::get_bindable_codes() )
	{
		const auto name = key_names::name_from_code( code );
		ASSERT_FALSE( name.empty() ) << "コード " << code << " に名前がない";
		EXPECT_EQ( code, key_names::code_from_name( name ) ) << name;
	}
}

TEST( KeyNamesTest, ESCは割り当てできない )
{
	EXPECT_FALSE( key_names::is_bindable( VK_ESCAPE ) );
	EXPECT_FALSE( key_names::is_bindable( VK_F1 ) );	// グローバルなホットキー
	EXPECT_FALSE( key_names::is_bindable( VK_F2 ) );
	EXPECT_FALSE( key_names::is_bindable( VK_F5 ) );
	EXPECT_TRUE( key_names::is_bindable( VK_F3 ) );
	EXPECT_FALSE( key_names::is_bindable( VK_SHIFT ) ); // 左右別のコードで割り当てる
	EXPECT_FALSE( key_names::is_bindable( VK_RETURN ) );	// 固定キー（決定）
	EXPECT_FALSE( key_names::is_bindable( VK_UP ) );		// 固定キー（方向）
	EXPECT_TRUE( key_names::is_bindable( VK_LSHIFT ) );
	EXPECT_TRUE( key_names::is_bindable( 'Q' ) );
	EXPECT_TRUE( key_names::is_bindable( VK_LBUTTON ) );
}

TEST( KeyNamesTest, パッドのボタンも名前とコードを行き来でき割り当てられる )
{
	using namespace blue_sky::key_names;

	const int pad3 = code_from_name( "PAD3" );
	EXPECT_TRUE( is_pad_code( pad3 ) );
	EXPECT_EQ( 2, pad_button_index( pad3 ) );
	EXPECT_EQ( "PAD3", name_from_code( pad3 ) );
	EXPECT_TRUE( is_bindable( pad3 ) );

	EXPECT_EQ( 0, code_from_name( "PAD0" ) );
	EXPECT_EQ( 0, code_from_name( "PAD33" ) );
}

// ---- 設定ファイルのキー割り当ての解釈（Input::parse_key_bindings_text）----

TEST( KeyBindingsTextTest, 2つ目以降に書いた固定キーは割り当てず理由をログに残す )
{
	std::vector< std::string > logs;
	const auto codes = blue_sky::Input::parse_key_bindings_text( "SPACE RETURN UP", [&logs] ( const std::string& s ) { logs.push_back( s ); } );

	ASSERT_EQ( 1u, codes.size() );
	EXPECT_EQ( VK_SPACE, codes[ 0 ] );
	ASSERT_EQ( 2u, logs.size() );
	EXPECT_NE( std::string::npos, logs[ 0 ].find( "RETURN" ) );
	EXPECT_NE( std::string::npos, logs[ 1 ].find( "UP" ) );
}

TEST( KeyBindingsTextTest, 解釈できない名前も飛ばしてログに残し重複は1つにする )
{
	std::vector< std::string > logs;
	const auto codes = blue_sky::Input::parse_key_bindings_text( "W NO_SUCH_KEY W MOUSE_L", [&logs] ( const std::string& s ) { logs.push_back( s ); } );

	ASSERT_EQ( 2u, codes.size() );
	EXPECT_EQ( 'W', codes[ 0 ] );
	EXPECT_EQ( VK_LBUTTON, codes[ 1 ] );
	ASSERT_EQ( 1u, logs.size() );
	EXPECT_NE( std::string::npos, logs[ 0 ].find( "NO_SUCH_KEY" ) );
}

TEST( KeyBindingsTextTest, 割り当てられるキーだけならログを出さない )
{
	std::vector< std::string > logs;
	const auto codes = blue_sky::Input::parse_key_bindings_text( "A LSHIFT PAD3", [&logs] ( const std::string& s ) { logs.push_back( s ); } );

	EXPECT_EQ( 3u, codes.size() );
	EXPECT_TRUE( logs.empty() );
}
