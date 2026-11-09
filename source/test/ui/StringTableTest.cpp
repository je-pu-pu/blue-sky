#include "pch.h"

#include <core/ui/StringTable.h>

using core::ui::StringTable;

TEST( StringTableTest, キーと値を読み込める )
{
	StringTable t;
	const int n = t.load_from_string( "# コメント\noptions.title = オプション\n\n  audio.volume=音量  \n" );

	EXPECT_EQ( 2, n );
	EXPECT_EQ( "オプション", t.get( "options.title" ) );
	EXPECT_EQ( "音量", t.get( "audio.volume" ) );
}

TEST( StringTableTest, 未登録のキーはキーそのものを返す )
{
	StringTable t;
	EXPECT_EQ( "no.such.key", t.get( "no.such.key" ) );
	EXPECT_FALSE( t.has( "no.such.key" ) );
}

TEST( StringTableTest, 不正な行は無視する )
{
	StringTable t;
	const int n = t.load_from_string( "イコール無し\n= 値だけ\nok = 1\r\n" );

	EXPECT_EQ( 1, n );
	EXPECT_EQ( "1", t.get( "ok" ) );
}

TEST( StringTableTest, BOMとCRLFと改行エスケープを扱える )
{
	StringTable t;
	t.load_from_string( "\xEF\xBB\xBF" "a = 一行目\\n二行目\r\nb = x=y\r\n" );

	EXPECT_EQ( "一行目\n二行目", t.get( "a" ) );
	EXPECT_EQ( "x=y", t.get( "b" ) ); // 最初の = で区切る
}

TEST( StringTableTest, 後から読み込んだ値で上書きする )
{
	StringTable t;
	t.load_from_string( "a = 1" );
	t.load_from_string( "a = 2" );
	EXPECT_EQ( "2", t.get( "a" ) );
}

TEST( StringTableTest, 番号付きプレースホルダを置換する )
{
	StringTable t;
	t.set( "dialog.keep", "残り {0} 秒 / {1} / {0}" );

	EXPECT_EQ( "残り 15 秒 / はい / 15", t.format( "dialog.keep", { "15", "はい" } ) );
}

TEST( StringTableTest, 範囲外や数字以外のプレースホルダはそのまま残す )
{
	EXPECT_EQ( "{2} {x} {} {", StringTable::format_text( "{2} {x} {} {", { "a" } ) );
}
