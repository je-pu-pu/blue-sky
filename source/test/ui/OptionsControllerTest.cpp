#include "pch.h"
#include "UITestHelper.h"

#include <Scene/OptionsController.h>

#include <core/setting/SettingStore.h>
#include <core/ui/Stack.h>
#include <core/ui/Button.h>

#include <game/Config.h>

using namespace core::ui;
using core::setting::SettingStore;
using core::setting::ApplyMode;
using blue_sky::OptionsController;

namespace
{

/// オプション画面の流れを、本物の設定ストアと画面で動かす土台
class OptionsControllerTest : public ::testing::Test, public ui_test::ScreenFixture
{
protected:
	SettingStore store;
	game::Config saved;			///< 最後に保存された内容
	int save_count = 0;
	int close_count = 0;
	float_t now = 0.f;

	std::unique_ptr< OptionsController > controller;

	void SetUp() override
	{
		store.add_int( "graphics.screen_width", 1920, 640, 7680, ApplyMode::DEFERRED );
		store.add_float( "audio.volume", 0.5f, 0.f, 1.f, 0.1f, ApplyMode::IMMEDIATE );

		// 元の解像度は既定値以外（既定値は保存されないため）
		store.set( "graphics.screen_width", 1600 );
		store.apply_deferred();

		auto root = std::make_unique< Stack >( Axis::VERTICAL );
		root->add< Button >( "back", nullptr );
		screen.set_root( std::move( root ), [] { } );

		OptionsController::Dependencies deps;
		deps.store = & store;
		deps.screen = & screen;
		deps.display_keys = { "graphics.screen_width" };
		deps.save = [this] { saved = game::Config(); store.save( saved ); save_count++; };
		deps.close = [this] { close_count++; };
		deps.now = [this] { return now; };
		deps.confirm_display_message = [] ( int remaining ) { return "keep? " + std::to_string( remaining ); };
		controller = std::make_unique< OptionsController >( std::move( deps ) );
	}

	string_t saved_width() const { return saved.get< string_t >( "graphics.screen_width", "" ); }

	/// ダイアログを開いた直後の入力抑止を抜ける
	void settle() { step(); step(); }
};

} // namespace

TEST_F( OptionsControllerTest, 表示が変わらない適用は確定して保存し閉じる )
{
	store.set( "audio.volume", 0.8f );
	controller->apply( true );

	EXPECT_EQ( 1, save_count );
	EXPECT_EQ( 1, close_count );
	EXPECT_EQ( "0.8", saved.get< string_t >( "audio.volume", "" ) );
	EXPECT_EQ( 1u, screen.get_layer_count() );		// ダイアログは出ない
}

TEST_F( OptionsControllerTest, 表示を変えたら確認中は元の値で保存し維持で新しい値を保存する )
{
	store.set( "graphics.screen_width", 1280 );
	controller->apply( true );
	settle();

	// 確認待ち : ファイルには映っていた元の値（強制終了しても次回は映る）
	EXPECT_EQ( "1600", saved_width() );
	EXPECT_EQ( 2u, screen.get_layer_count() );
	EXPECT_EQ( 0, close_count );

	// 維持（既定のボタン）
	confirm();

	EXPECT_EQ( "1280", saved_width() );
	EXPECT_EQ( 1, close_count );
	EXPECT_FALSE( store.in_trial() );
}

TEST_F( OptionsControllerTest, 時間内に維持しなければ元に戻して保存する )
{
	store.set( "graphics.screen_width", 1280 );
	controller->apply( false );
	settle();

	// 時間切れまで進める（ダイアログは自分の経過時間で数える）
	for ( int i = 0; i < 20; i++ )
	{
		now += 1.f;
		step( UIInput(), 1.f );
	}

	EXPECT_EQ( 1600, store.get< int >( "graphics.screen_width" ) );
	EXPECT_EQ( "1600", saved_width() );
	EXPECT_FALSE( store.in_trial() );
	EXPECT_EQ( 1u, screen.get_layer_count() );
	EXPECT_EQ( 0, close_count );		// 適用だけ（閉じない）
}

TEST_F( OptionsControllerTest, 未適用の変更があるまま戻ると確認し破棄を選べば元に戻して閉じる )
{
	store.set( "graphics.screen_width", 1280 );		// 未適用
	controller->back();
	settle();

	EXPECT_EQ( 2u, screen.get_layer_count() );

	// 「破棄して戻る」（2 番目）
	press_nav( NavDir::RIGHT );
	confirm();

	EXPECT_EQ( 1600, store.get< int >( "graphics.screen_width" ) );
	EXPECT_EQ( 1, close_count );
}

TEST_F( OptionsControllerTest, 未適用の変更が無ければ確認せずに確定して閉じる )
{
	store.set( "audio.volume", 0.9f );		// 即時反映の項目だけ
	controller->back();

	EXPECT_EQ( 1u, screen.get_layer_count() );
	EXPECT_EQ( 1, close_count );
	EXPECT_EQ( "0.9", saved.get< string_t >( "audio.volume", "" ) );
}
