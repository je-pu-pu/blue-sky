#include "pch.h"

#include <core/setting/SettingStore.h>
#include <game/Config.h>

using core::setting::SettingStore;
using core::setting::ApplyMode;

namespace
{

/// 反映関数の呼び出しを記録する
struct ApplyLog
{
	std::vector< SettingStore::Value > values;
	SettingStore::ApplyFn fn() { return [this] ( const SettingStore::Value& v ) { values.push_back( v ); }; }
};

} // namespace

TEST( SettingStoreTest, 範囲外はクランプされ量子化される )
{
	SettingStore s;
	s.add_float( "audio.volume", 0.5f, 0.f, 1.f, 0.1f, ApplyMode::IMMEDIATE );
	s.add_int( "camera.fov", 90, 50, 120, ApplyMode::IMMEDIATE );

	s.set( "audio.volume", 1.7f );
	EXPECT_FLOAT_EQ( 1.f, s.get< float_t >( "audio.volume" ) );

	s.set( "audio.volume", 0.34f );
	EXPECT_NEAR( 0.3f, s.get< float_t >( "audio.volume" ), 1e-5f );

	s.set( "camera.fov", 10 );
	EXPECT_EQ( 50, s.get< int >( "camera.fov" ) );

	s.set( "camera.fov", 99.6f ); // float を int 項目へ入れると丸める
	EXPECT_EQ( 100, s.get< int >( "camera.fov" ) );
}

TEST( SettingStoreTest, 浮動小数の誤差では変更ありにならない )
{
	SettingStore s;
	s.add_float( "v", 0.3f, 0.f, 1.f, 0.1f, ApplyMode::IMMEDIATE );

	// 0.1 を 3 回足して 3 回引く（誤差が出る操作）
	float_t v = s.get< float_t >( "v" );
	for ( int i = 0; i < 3; i++ ) { v += 0.1f; s.set( "v", v ); v = s.get< float_t >( "v" ); }
	for ( int i = 0; i < 3; i++ ) { v -= 0.1f; s.set( "v", v ); v = s.get< float_t >( "v" ); }

	EXPECT_FALSE( s.has_pending_changes() );
}

TEST( SettingStoreTest, IMMEDIATEは変更時に反映され値が変わらなければ呼ばれない )
{
	ApplyLog log;
	SettingStore s;
	s.add_bool( "audio.mute", false, ApplyMode::IMMEDIATE, log.fn() );

	s.set( "audio.mute", true );
	s.set( "audio.mute", true );

	ASSERT_EQ( 1u, log.values.size() );
	EXPECT_EQ( SettingStore::Value( true ), log.values[ 0 ] );
	EXPECT_TRUE( s.has_pending_changes() );
	EXPECT_FALSE( s.has_pending_deferred() );

	s.commit_immediate();
	EXPECT_FALSE( s.has_pending_changes() );
	EXPECT_TRUE( std::get< bool >( s.get_committed( "audio.mute" ) ) );
}

TEST( SettingStoreTest, DEFERREDはapply_deferredまで反映されない )
{
	ApplyLog log;
	SettingStore s;
	s.add_bool( "graphics.full_screen", false, ApplyMode::DEFERRED, log.fn() );

	s.set( "graphics.full_screen", true );
	EXPECT_TRUE( log.values.empty() );
	EXPECT_TRUE( s.has_pending_deferred() );

	EXPECT_TRUE( s.apply_deferred() );
	EXPECT_EQ( 1u, log.values.size() );
	EXPECT_FALSE( s.has_pending_deferred() );

	EXPECT_FALSE( s.apply_deferred() ); // 変化なし
	EXPECT_EQ( 1u, log.values.size() );
}

TEST( SettingStoreTest, グループは複数キーが変わっても1回だけ反映する )
{
	int group_calls = 0;
	ApplyLog individual;
	SettingStore s;
	s.add_int( "graphics.screen_width", 1920, 320, 7680, ApplyMode::DEFERRED, individual.fn() );
	s.add_int( "graphics.screen_height", 1080, 240, 4320, ApplyMode::DEFERRED, individual.fn() );
	s.add_apply_group( { "graphics.screen_width", "graphics.screen_height" }, [&] { group_calls++; } );

	s.set( "graphics.screen_width", 1280 );
	s.set( "graphics.screen_height", 720 );
	s.apply_deferred();

	EXPECT_EQ( 1, group_calls );
	EXPECT_TRUE( individual.values.empty() ); // グループ所属の個別反映は呼ばれない
}

TEST( SettingStoreTest, revertはIMMEDIATEを確定値で反映し直す )
{
	ApplyLog log;
	SettingStore s;
	s.add_float( "audio.volume", 0.5f, 0.f, 1.f, 0.1f, ApplyMode::IMMEDIATE, log.fn() );
	s.add_bool( "graphics.full_screen", false, ApplyMode::DEFERRED );

	s.set( "audio.volume", 0.8f );
	s.set( "graphics.full_screen", true );
	s.revert();

	EXPECT_FALSE( s.has_pending_changes() );
	ASSERT_EQ( 2u, log.values.size() );
	EXPECT_NEAR( 0.5f, std::get< float_t >( log.values[ 1 ] ), 1e-5f );
}

TEST( SettingStoreTest, revert_deferredはIMMEDIATEに触れない )
{
	SettingStore s;
	s.add_float( "audio.volume", 0.5f, 0.f, 1.f, 0.1f, ApplyMode::IMMEDIATE );
	s.add_bool( "graphics.full_screen", false, ApplyMode::DEFERRED );

	s.set( "audio.volume", 0.8f );
	s.set( "graphics.full_screen", true );
	s.revert_deferred();

	EXPECT_FALSE( s.get< bool >( "graphics.full_screen" ) );
	EXPECT_NEAR( 0.8f, s.get< float_t >( "audio.volume" ), 1e-5f );
}

TEST( SettingStoreTest, 既定値に戻すのは保留値だけ )
{
	ApplyLog log;
	SettingStore s;
	s.add_int( "camera.fov", 90, 50, 120, ApplyMode::IMMEDIATE, log.fn() );
	s.set( "camera.fov", 70 );
	s.commit_immediate();

	s.reset_to_defaults();

	EXPECT_EQ( 90, s.get< int >( "camera.fov" ) );
	EXPECT_EQ( 70, std::get< int >( s.get_committed( "camera.fov" ) ) );
	EXPECT_EQ( 2u, log.values.size() ); // 70 と 90 で 2 回反映
}

TEST( SettingStoreTest, スナップショットでロールバックできる )
{
	int group_calls = 0;
	SettingStore s;
	s.add_int( "graphics.screen_width", 1920, 320, 7680, ApplyMode::DEFERRED );
	s.add_int( "graphics.screen_height", 1080, 240, 4320, ApplyMode::DEFERRED );
	s.add_float( "audio.volume", 0.5f, 0.f, 1.f, 0.1f, ApplyMode::IMMEDIATE );
	s.add_apply_group( { "graphics.screen_width", "graphics.screen_height" }, [&] { group_calls++; } );

	const auto snap = s.snapshot();
	s.set( "graphics.screen_width", 1280 );
	s.set( "audio.volume", 0.9f );
	s.apply_deferred();
	s.commit_immediate();
	EXPECT_EQ( 1, group_calls );

	// 表示系だけ戻す（音量は確定のまま）
	s.rollback( snap, { "graphics.screen_width", "graphics.screen_height" } );

	EXPECT_EQ( 2, group_calls );
	EXPECT_EQ( 1920, s.get< int >( "graphics.screen_width" ) );
	EXPECT_EQ( 1920, std::get< int >( s.get_committed( "graphics.screen_width" ) ) );
	EXPECT_NEAR( 0.9f, std::get< float_t >( s.get_committed( "audio.volume" ) ), 1e-5f );
	EXPECT_FALSE( s.has_pending_changes() );
}

TEST( SettingStoreTest, on_setフックで他の項目を入れ替えられる )
{
	SettingStore s;
	s.add_string( "input.key.left", "A", ApplyMode::IMMEDIATE );
	s.add_string( "input.key.right", "D", ApplyMode::IMMEDIATE );

	// 同じキーを使っている他の項目に旧値を渡す
	auto swap_hook = [] ( SettingStore& store, const string_t& key, const SettingStore::Value& old_value )
	{
		const auto now = store.get< string_t >( key );

		for ( const char_t* other : { "input.key.left", "input.key.right" } )
		{
			if ( key != other && store.get< string_t >( other ) == now )
			{
				store.set( other, old_value );
			}
		}
	};

	s.set_on_set( "input.key.left", swap_hook );
	s.set_on_set( "input.key.right", swap_hook );

	s.set( "input.key.left", "D" );

	EXPECT_EQ( "D", s.get< string_t >( "input.key.left" ) );
	EXPECT_EQ( "A", s.get< string_t >( "input.key.right" ) );
}

TEST( SettingStoreTest, 外部変更の取り込みは反映関数を呼ばず保留も作らない )
{
	ApplyLog log;
	SettingStore s;
	s.add_bool( "audio.mute", false, ApplyMode::IMMEDIATE, log.fn() );

	const auto count = s.get_change_count();
	s.sync_from_runtime( "audio.mute", true );

	EXPECT_TRUE( s.get< bool >( "audio.mute" ) );
	EXPECT_FALSE( s.has_pending_changes() );
	EXPECT_TRUE( log.values.empty() );
	EXPECT_GT( s.get_change_count(), count );
}

TEST( SettingStoreTest, 設定ファイルの読み書き )
{
	SettingStore s;
	s.add_bool( "audio.mute", false, ApplyMode::IMMEDIATE );
	s.add_float( "audio.volume", 0.5f, 0.f, 1.f, 0.1f, ApplyMode::IMMEDIATE );
	s.add_int( "camera.fov", 90, 50, 120, ApplyMode::IMMEDIATE );
	s.add_string( "input.key.left", "A", ApplyMode::IMMEDIATE );

	game::Config config;
	config.read_line( "audio.mute 1" );			// 従来形式の 0/1
	config.read_line( "audio.volume 0.3" );
	config.read_line( "camera.fov 500" );		// 範囲外 → クランプ
	config.read_line( "input.key.left W LEFT" );	// 空白を含む値

	s.load( config );

	EXPECT_TRUE( s.get< bool >( "audio.mute" ) );
	EXPECT_NEAR( 0.3f, s.get< float_t >( "audio.volume" ), 1e-5f );
	EXPECT_EQ( 120, s.get< int >( "camera.fov" ) );
	EXPECT_EQ( "W LEFT", s.get< string_t >( "input.key.left" ) );
	EXPECT_FALSE( s.has_pending_changes() );

	game::Config out;
	s.save( out );

	EXPECT_EQ( "1", out.get< string_t >( "audio.mute", "" ) );
	EXPECT_EQ( "0.3", out.get< string_t >( "audio.volume", "" ) );
	EXPECT_EQ( "120", out.get< string_t >( "camera.fov", "" ) );
	EXPECT_EQ( "W LEFT", out.get< string_t >( "input.key.left", "" ) );
}

TEST( SettingStoreTest, 壊れた値は既定値になる )
{
	SettingStore s;
	s.add_float( "audio.volume", 0.5f, 0.f, 1.f, 0.1f, ApplyMode::IMMEDIATE );
	s.add_bool( "audio.mute", false, ApplyMode::IMMEDIATE );

	game::Config config;
	config.read_line( "audio.volume abc" );
	config.read_line( "audio.mute 1x" );

	s.load( config );

	EXPECT_NEAR( 0.5f, s.get< float_t >( "audio.volume" ), 1e-5f );
	EXPECT_FALSE( s.get< bool >( "audio.mute" ) );
}

TEST( SettingStoreTest, 未登録キーと型違いの操作は無視する )
{
	SettingStore s;
	s.add_int( "camera.fov", 90, 50, 120, ApplyMode::IMMEDIATE );

	s.set( "no.such.key", 1 );
	s.set( "camera.fov", "abc" );

	EXPECT_FALSE( s.has( "no.such.key" ) );
	EXPECT_EQ( 0, s.get< int >( "no.such.key" ) );
	EXPECT_EQ( 90, s.get< int >( "camera.fov" ) );
}
