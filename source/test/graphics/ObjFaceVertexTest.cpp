#include "pch.h"

#include <blue_sky/graphics/ObjFileLoader.h>

using blue_sky::graphics::obj::parse_face_vertex;

TEST( ObjFaceVertexTest, 位置_UV_法線をすべて書いた頂点 )
{
	const auto v = parse_face_vertex( "3/2/1" );
	ASSERT_TRUE( v );
	EXPECT_EQ( 2u, v->position );
	EXPECT_EQ( 1u, v->tex_coord.value() );
	EXPECT_EQ( 0u, v->normal.value() );
}

TEST( ObjFaceVertexTest, UV_の無い頂点は法線の番号を_UV_として読まない )
{
	// 以前は "1//3" の 3 を UV の番号として読んでいた
	const auto v = parse_face_vertex( "1//3" );
	ASSERT_TRUE( v );
	EXPECT_EQ( 0u, v->position );
	EXPECT_FALSE( v->tex_coord );
	EXPECT_EQ( 2u, v->normal.value() );
}

TEST( ObjFaceVertexTest, 位置だけ_位置と_UV_だけの頂点 )
{
	const auto a = parse_face_vertex( "5" );
	ASSERT_TRUE( a );
	EXPECT_EQ( 4u, a->position );
	EXPECT_FALSE( a->tex_coord );
	EXPECT_FALSE( a->normal );

	const auto b = parse_face_vertex( "5/6" );
	ASSERT_TRUE( b );
	EXPECT_EQ( 5u, b->tex_coord.value() );
	EXPECT_FALSE( b->normal );
}

TEST( ObjFaceVertexTest, 正しくない書き方は_nullopt )
{
	EXPECT_FALSE( parse_face_vertex( "" ) );
	EXPECT_FALSE( parse_face_vertex( "0" ) );			// 番号は 1 から
	EXPECT_FALSE( parse_face_vertex( "-1" ) );			// 負の（後ろからの）番号は扱わない
	EXPECT_FALSE( parse_face_vertex( "a/1" ) );
	EXPECT_FALSE( parse_face_vertex( "1/2/3/4" ) );
	EXPECT_FALSE( parse_face_vertex( "/2/3" ) );		// 位置は必須
	EXPECT_FALSE( parse_face_vertex( "1/0" ) );
}
