#include "pch.h"

#include <core/setting/SettingStore.h>
#include <game/Config.h>
#include <blue_sky/GameSettings.h>

#include <windows.h>

#include <filesystem>
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

TEST( SettingStoreTest, 試用中は維持するまで元の値を保存する )
{
	SettingStore s;
	s.add_int( "graphics.screen_width", 1920, 320, 7680, ApplyMode::DEFERRED );
	s.add_float( "audio.volume", 0.5f, 0.f, 1.f, 0.1f, ApplyMode::IMMEDIATE );

	// 元の値は既定値以外にしておく（既定値は保存されないため）
	s.set( "graphics.screen_width", 1600 );
	s.apply_deferred();

	const auto snap = s.snapshot();
	s.set( "graphics.screen_width", 1280 );
	s.set( "audio.volume", 0.9f );
	s.apply_deferred();
	s.commit_immediate();
	s.begin_trial( snap, { "graphics.screen_width" } );
	EXPECT_TRUE( s.in_trial() );

	// 確認待ちの間に保存（強制終了の直前など）: 表示は元の値、音量は新しい値
	game::Config during;
	s.save( during );
	EXPECT_EQ( "1600", during.get< string_t >( "graphics.screen_width", "" ) );
	EXPECT_EQ( "0.9", during.get< string_t >( "audio.volume", "" ) );

	// 維持したら新しい値を保存する
	s.end_trial();
	EXPECT_FALSE( s.in_trial() );

	game::Config after;
	s.save( after );
	EXPECT_EQ( "1280", after.get< string_t >( "graphics.screen_width", "" ) );
}

TEST( SettingStoreTest, 試用はロールバックでも終わる )
{
	SettingStore s;
	s.add_int( "graphics.screen_width", 1920, 320, 7680, ApplyMode::DEFERRED );

	const auto snap = s.snapshot();
	s.set( "graphics.screen_width", 1280 );
	s.apply_deferred();
	s.begin_trial( snap, { "graphics.screen_width" } );

	s.rollback( snap, { "graphics.screen_width" } );
	EXPECT_FALSE( s.in_trial() );

	game::Config out;
	out.set< string_t >( "graphics.screen_width", "1280" );		// 以前に保存されていた値
	s.save( out );
	EXPECT_FALSE( out.has( "graphics.screen_width" ) );			// 既定値に戻ったので消える
}

TEST( SettingStoreTest, 購読すると反映される値が変わったときだけ知らされ破棄で止まる )
{
	SettingStore s;
	s.add_float( "camera.fov", 90.f, 50.f, 120.f, 1.f, ApplyMode::IMMEDIATE );
	s.add_int( "graphics.screen_width", 1920, 640, 7680, ApplyMode::DEFERRED );

	std::vector< float_t > fov;
	std::vector< int > width;

	{
		auto fov_sub = s.subscribe( "camera.fov", [&] ( const SettingStore::Value& v ) { fov.push_back( std::get< float_t >( v ) ); } );
		auto width_sub = s.subscribe( "graphics.screen_width", [&] ( const SettingStore::Value& v ) { width.push_back( std::get< int >( v ) ); } );

		s.set( "camera.fov", 100.f );		// 即時反映 : すぐ届く
		s.set( "camera.fov", 100.f );		// 変わらない : 届かない
		s.set( "graphics.screen_width", 1280 );	// 適用まで反映しない : まだ届かない
		EXPECT_TRUE( width.empty() );

		s.apply_deferred();					// 反映 : 届く
	}

	s.set( "camera.fov", 110.f );			// 購読を破棄した後 : 届かない

	ASSERT_EQ( 1u, fov.size() );
	EXPECT_FLOAT_EQ( 100.f, fov[ 0 ] );
	ASSERT_EQ( 1u, width.size() );
	EXPECT_EQ( 1280, width[ 0 ] );
}

TEST( SettingStoreTest, 保存は既定値と違う項目だけを書き版を付ける )
{
	SettingStore s;
	s.add_float( "audio.volume", 0.5f, 0.f, 1.f, 0.1f, ApplyMode::IMMEDIATE );
	s.add_bool( "audio.mute", false, ApplyMode::IMMEDIATE );

	s.set( "audio.volume", 0.8f );
	s.commit_immediate();

	game::Config out;
	out.set< string_t >( "audio.mute", "1" );		// 以前は既定値以外だった
	s.save( out, 3 );

	EXPECT_EQ( "0.8", out.get< string_t >( "audio.volume", "" ) );
	EXPECT_FALSE( out.has( "audio.mute" ) );
	EXPECT_EQ( 3, out.get< int >( SettingStore::VERSION_KEY, 0 ) );
}

TEST( SettingStoreTest, 型付きのキーで読み書きでき範囲と情報を引ける )
{
	constexpr core::setting::SettingKey< float_t > VOLUME { "audio.volume" };
	constexpr core::setting::SettingKey< int > WIDTH { "graphics.screen_width" };

	SettingStore s;
	s.add_float( VOLUME, 0.5f, 0.f, 1.f, 0.05f, ApplyMode::IMMEDIATE );
	s.add_int( WIDTH, 1920, 640, 7680, ApplyMode::DEFERRED );
	s.set_info( VOLUME, "options.volume", "options.volume.description" );

	s.set( VOLUME, 0.25 );		// double を渡しても項目の型（float）で受ける
	EXPECT_FLOAT_EQ( 0.25f, s.get( VOLUME ) );

	s.set( WIDTH, 1280 );
	s.apply_deferred();
	EXPECT_EQ( 1280, s.get_committed( WIDTH ) );

	SettingStore::Range range;
	ASSERT_TRUE( s.get_range( VOLUME, range ) );
	EXPECT_FLOAT_EQ( 0.f, range.min );
	EXPECT_FLOAT_EQ( 1.f, range.max );
	EXPECT_FLOAT_EQ( 0.05f, range.step );

	EXPECT_EQ( "options.volume", s.get_info( VOLUME ).label_key );
	EXPECT_EQ( "options.volume.description", s.get_info( VOLUME ).description_key );
}

TEST( SettingStoreTest, 未登録のキーと型の違いは誤りとして知らせる )
{
	SettingStore s;
	s.add_float( "audio.volume", 0.5f, 0.f, 1.f, 0.1f, ApplyMode::IMMEDIATE );

	std::vector< string_t > errors;
	s.set_error_handler( [&] ( const string_t& m ) { errors.push_back( m ); } );

	s.get< float_t >( "audio.volum" );			// 打ち間違い
	s.get< int >( "audio.volume" );				// 型の違い
	s.set( "audio.volume", "loud" );			// 文字列と数値の取り違え

	EXPECT_EQ( 3u, errors.size() );
	EXPECT_FLOAT_EQ( 0.5f, s.get< float_t >( "audio.volume" ) );
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

TEST( ConfigTest, 値の無い行とコメントと余分な空白を読める )
{
	game::Config config;
	config.read_line( "debug.start_overlay" );			// 値が無い（以前は例外で起動できなかった）
	config.read_line( "# コメント" );
	config.read_line( "" );
	config.read_line( "   input.key.left   W    LEFT   " );

	EXPECT_EQ( "", config.get< string_t >( "debug.start_overlay", "x" ) );
	EXPECT_EQ( "W LEFT", config.get< string_t >( "input.key.left", "" ) );
	EXPECT_EQ( "", static_cast< const game::Config& >( config ).get< string_t >( "#", "" ) );
}

TEST( ConfigTest, 保存は一時ファイル経由で置き換える )
{
	const std::filesystem::path dir = std::filesystem::temp_directory_path() / "blue-sky-config-test";
	std::filesystem::create_directories( dir );
	const std::filesystem::path file = dir / "test.config";
	const std::string path = file.string();

	game::Config a;
	a.read_line( "audio.volume 0.3" );
	ASSERT_TRUE( a.save_file( path.c_str() ) );

	game::Config b;
	b.read_line( "audio.volume 0.7" );
	ASSERT_TRUE( b.save_file( path.c_str() ) );		// 既存のファイルを置き換える

	EXPECT_FALSE( std::filesystem::exists( path + ".tmp" ) );

	game::Config loaded;
	ASSERT_TRUE( loaded.load_file( path.c_str() ) );
	EXPECT_EQ( "0.7", loaded.get< string_t >( "audio.volume", "" ) );

	std::filesystem::remove_all( dir );
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

TEST( UserSettingsPathTest, ユーザーのフォルダに置き環境変数で置き場所を変えられる )
{
	// 既定 : %LOCALAPPDATA%/blue-sky/settings.config（作業ディレクトリではない）
	SetEnvironmentVariableW( L"BLUE_SKY_SETTINGS_DIR", nullptr );
	const std::filesystem::path def = blue_sky::game_settings::user_settings_path();
	EXPECT_EQ( "settings.config", def.filename().string() );
	EXPECT_EQ( "blue-sky", def.parent_path().filename().string() );
	EXPECT_TRUE( def.is_absolute() );

	// 環境変数で指定したフォルダ
	SetEnvironmentVariableW( L"BLUE_SKY_SETTINGS_DIR", L"C:/temp/bs-test" );
	EXPECT_EQ( std::filesystem::path( "C:/temp/bs-test" ) / "settings.config", std::filesystem::path( blue_sky::game_settings::user_settings_path() ) );
	SetEnvironmentVariableW( L"BLUE_SKY_SETTINGS_DIR", nullptr );
}

TEST( SettingStoreTest, 値を整える関数は設定と読み込みと元に戻すと既定値に戻すの全部で呼ばれる )
{
	SettingStore s;
	s.add_int( "graphics.screen_width", 1920, 640, 7680, ApplyMode::DEFERRED );
	s.add_int( "volume", 5, 0, 10, ApplyMode::IMMEDIATE );

	int calls = 0;
	s.set_normalizer( [&] ( SettingStore& store )
	{
		calls++;
		// 例 : 音量は奇数にしない（中で set しても入れ子では呼ばれない）
		if ( store.get< int >( "volume" ) % 2 == 1 ) store.set( "volume", store.get< int >( "volume" ) + 1 );
	} );

	s.set( "volume", 3 );
	EXPECT_EQ( 4, s.get< int >( "volume" ) );
	EXPECT_EQ( 1, calls );

	game::Config c;
	c.set< string_t >( "volume", "7" );
	s.load( c );
	EXPECT_EQ( 8, s.get< int >( "volume" ) );		// 読み込みでも整える（以前は set のときしか検査しなかった）

	const int before = calls;
	s.revert();
	s.revert_deferred();
	s.reset_to_defaults();
	EXPECT_EQ( 6, s.get< int >( "volume" ) );		// 既定値 5 も整える
	s.rollback( s.snapshot(), { "volume" } );		// スナップショットは確定値（8）
	EXPECT_EQ( before + 4, calls );
}

TEST( SettingStoreTest, 操作を終えたら反映する項目はapply_pendingか確定まで反映しない )
{
	SettingStore s;
	float applied = 0.f;
	s.add_float( "ui.text_scale", 1.f, 0.8f, 1.6f, 0.1f, ApplyMode::ON_RELEASE, [&] ( const SettingStore::Value& v ) { applied = std::get< float_t >( v ); } );

	s.set( "ui.text_scale", 1.2f );
	s.set( "ui.text_scale", 1.4f );		// ドラッグの途中
	EXPECT_FLOAT_EQ( 0.f, applied );
	EXPECT_FALSE( s.has_pending_deferred() );		// 「適用」の確認の対象ではない

	s.apply_pending( "ui.text_scale" );		// 離した
	EXPECT_FLOAT_EQ( 1.4f, applied );

	s.set( "ui.text_scale", 1.6f );
	s.commit_immediate();					// 反映せずに戻ろうとしても、反映してから確定する
	EXPECT_FLOAT_EQ( 1.6f, applied );
}
