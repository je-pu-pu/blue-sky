#include "pch.h"

#include <core/ResourceManager.h>

namespace
{

/// 解放されたことを数えるリソース
struct Counted
{
	static inline int alive = 0;
	int value;

	explicit Counted( int v ) : value( v ) { alive++; }
	~Counted() { alive--; }
};

} // namespace

TEST( ResourceManagerTest, 同じ名前で登録し直すと名前は新しいものを指し古いものは使い続けられる )
{
	Counted::alive = 0;
	core::ResourceManager< Counted > m;

	Counted* a = m.create_named( "a", 1 );
	const auto ha = m.handle_of( a );
	EXPECT_EQ( a, m.resolve( ha ) );

	m.create_named( "a", 2 );		// 差し替え（シーンごとにシェーダーを作り直すのと同じ）

	EXPECT_EQ( 2, Counted::alive );					// 古いものは生ポインタで使われ得るので解放しない
	EXPECT_EQ( 1, a->value );
	EXPECT_EQ( a, m.resolve( ha ) );				// 古いハンドルも有効なまま
	EXPECT_EQ( 2, m.get( "a" )->value );
	EXPECT_EQ( 2, m.resolve( m.get_handle( "a" ) )->value );
}

TEST( ResourceManagerTest, 別の名前でも指されているリソースは名前を消しても解放しない )
{
	Counted::alive = 0;
	core::ResourceManager< Counted > m;

	Counted* r = new Counted( 7 );
	m.add_named( "x", r );
	m.add_named( "y", r );		// 同じリソースに 2 つ目の名前
	EXPECT_EQ( 1u, m.get_resource_list().size() );

	m.remove_named( "x" );
	EXPECT_EQ( 1, Counted::alive );		// y がまだ指している
	EXPECT_EQ( 7, m.get( "y" )->value );

	const auto h = m.get_handle( "y" );
	m.remove_named( "y" );
	EXPECT_EQ( 0, Counted::alive );
	EXPECT_EQ( nullptr, m.resolve( h ) );

	m.remove_named( "none" );			// 無い名前は何もしない
	m.add_named( "null", nullptr );		// nullptr は登録しない
	EXPECT_EQ( nullptr, m.get( "null" ) );
}

TEST( ResourceManagerTest, 全部消すとハンドルはすべて無効で番号は使い回さない )
{
	Counted::alive = 0;
	core::ResourceManager< Counted > m;

	const auto h1 = m.handle_of( m.create< Counted >( 1 ) );
	m.clear_all();
	EXPECT_EQ( 0, Counted::alive );
	EXPECT_EQ( nullptr, m.resolve( h1 ) );

	const auto h2 = m.handle_of( m.create< Counted >( 2 ) );
	EXPECT_NE( h1, h2 );					// 同じ番号を使い回さない（古いハンドルが新しいものを指さない）
	EXPECT_EQ( nullptr, m.resolve( h1 ) );
	EXPECT_TRUE( core::ResourceManager< Counted >::Handle().is_null() );
	EXPECT_EQ( nullptr, m.resolve( core::ResourceManager< Counted >::Handle() ) );
}

TEST( ResourceManagerTest, 作った側が手放すとそれを指す名前も外れて解放される )
{
	Counted::alive = 0;
	core::ResourceManager< Counted > m;

	Counted* a = m.create_named( "a", 1 );
	m.add_named( "a2", a );
	const auto ha = m.handle_of( a );
	Counted* b = m.create( 2 );

	m.release( a );		// 作った側（CityGenerator など）が使い終わった

	EXPECT_EQ( 1, Counted::alive );
	EXPECT_EQ( nullptr, m.get( "a" ) );
	EXPECT_EQ( nullptr, m.get( "a2" ) );
	EXPECT_EQ( nullptr, m.resolve( ha ) );			// ハンドルも無効になる
	EXPECT_EQ( 2, b->value );						// ほかのリソースはそのまま
}

TEST( ResourceManagerTest, 差し替えると同じハンドルで新しい中身が引け古い中身は解放される )
{
	Counted::alive = 0;
	core::ResourceManager< Counted > m;

	Counted* a = m.create_named( "a", 1 );
	m.add_named( "a2", a );
	const auto h = m.get_handle( "a" );

	EXPECT_TRUE( m.replace( h, std::make_unique< Counted >( 2 ) ) );

	EXPECT_EQ( 1, Counted::alive );					// 古い中身は解放された
	EXPECT_EQ( 2, m.resolve( h )->value );			// 番号は変わらず、新しい中身を指す
	EXPECT_EQ( h, m.get_handle( "a" ) );
	EXPECT_EQ( 2, m.get( "a" )->value );			// 名前も新しい中身を指す
	EXPECT_EQ( 2, m.get( "a2" )->value );
	EXPECT_EQ( h, m.handle_of( m.resolve( h ) ) );
	EXPECT_EQ( 1u, m.get_resource_list().size() );

	m.remove_named( "a" );
	m.remove_named( "a2" );
	EXPECT_EQ( 0, Counted::alive );
	EXPECT_EQ( nullptr, m.resolve( h ) );			// 解放後は nullptr
}

TEST( ResourceManagerTest, 無効なハンドルへの差し替えは失敗し新しい中身は解放される )
{
	Counted::alive = 0;
	core::ResourceManager< Counted > m;

	EXPECT_FALSE( m.replace( core::ResourceManager< Counted >::Handle(), std::make_unique< Counted >( 1 ) ) );
	EXPECT_EQ( 0, Counted::alive );

	const auto h = m.handle_of( m.create_named( "a", 1 ) );
	m.remove_named( "a" );
	EXPECT_FALSE( m.replace( h, std::make_unique< Counted >( 2 ) ) );	// 解放済み
	EXPECT_EQ( 0, Counted::alive );
	EXPECT_EQ( nullptr, m.resolve( h ) );

	Counted* b = m.create( 3 );
	const auto hb = m.handle_of( b );
	EXPECT_FALSE( m.replace( hb, nullptr ) );
	EXPECT_FALSE( m.replace( hb, std::unique_ptr< Counted >( b ) ) );	// すでに管理しているものは引き取らない（二重に解放しない）
	EXPECT_EQ( 1, Counted::alive );
	EXPECT_EQ( b, m.resolve( hb ) );
}
