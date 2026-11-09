#include "pch.h"
#include "UITestHelper.h"

#include <core/ui/Button.h>
#include <core/ui/Choice.h>
#include <core/ui/Dialog.h>
#include <core/ui/Grid.h>
#include <core/ui/Stack.h>
#include <core/ui/TextField.h>
#include <core/ui/TextFit.h>

#include <Scene/SceneRequest.h>

using namespace core::ui;
using ui_test::ScreenFixture;

namespace
{

/// 指定のアクションを受け取って数えるコンテナ（ほかは基底に任せる）
class ActionCatcher : public Stack
{
public:
	ActionId want;
	int count = 0;

	explicit ActionCatcher( ActionId a ) : Stack( Axis::VERTICAL ), want( a ) { }

	bool on_action( ActionId a, UIContext& ctx ) override
	{
		if ( a == want )
		{
			count++;
			return true;
		}

		return Stack::on_action( a, ctx );
	}
};

/// 捕捉の解除を数えるボタン
class CountingButton : public Button
{
public:
	int lost = 0;

	using Button::Button;

	void on_capture_lost() override
	{
		lost++;
		Button::on_capture_lost();
	}
};

/// 独自の操作のヒントを出すボタン
class ResetHintButton : public Button
{
public:
	using Button::Button;

	void collect_hints( std::vector< Hint >& hints ) const override
	{
		hints.push_back( Hint{ ui_action::USER + 1, "hint.reset_row" } );
	}
};

class ActionTest : public ::testing::Test, public ScreenFixture
{
protected:
	Stack* root = nullptr;

	void SetUp() override
	{
		auto r = std::make_unique< Stack >( Axis::VERTICAL );
		root = r.get();
		screen.set_root( std::move( r ), [] { } );
	}

	void ready()
	{
		screen.invalidate_layout();
		step();
		screen.focus_into( root );
	}
};

} // namespace

TEST_F( ActionTest, 子が使わなかった独自の操作は親に届く )
{
	constexpr ActionId RESET = ui_action::USER + 1;

	auto* catcher = root->add< ActionCatcher >( RESET );
	catcher->add< Button >( "ok", nullptr );
	ready();

	UIInput in;
	in.actions.push_back( RESET );
	step( in );

	EXPECT_EQ( 1, catcher->count );
}

TEST_F( ActionTest, 組み込みの操作も親へ回り親が使えば既定の処理は行わない )
{
	// 親が「下」を使う（リストのページ送りなど）: フォーカスは動かない
	auto* catcher = root->add< ActionCatcher >( ui_action::NAV_DOWN );
	auto* first = catcher->add< Button >( "a", nullptr );
	catcher->add< Button >( "b", nullptr );
	ready();

	press_nav( NavDir::DOWN );

	EXPECT_EQ( 1, catcher->count );
	EXPECT_TRUE( first->is_focused() );
}

TEST_F( ActionTest, 独自の操作のヒントは登録した名前で記号を引く )
{
	strings.set( "hint.key.reset_row", "R" );
	strings.set( "hint.reset_row", "既定値に戻す" );
	screen.register_action( ui_action::USER + 1, "reset_row" );

	root->add< ResetHintButton >( "ok", nullptr );
	ready();

	EXPECT_NE( string_t::npos, screen.build_hint_text().find( "R 既定値に戻す" ) );
}

TEST_F( ActionTest, 入力欄は文字を受け取りバックスペースで1文字消す )
{
	string_t value;
	root->add< TextField >( "name", [&] { return value; }, [&] ( const string_t& v ) { value = v; } );
	ready();

	UIInput in;
	in.text = U"あb";
	step( in );
	EXPECT_EQ( "あb", value );

	in.text = U"\b\b";
	step( in );
	EXPECT_EQ( "", value );

	in.text = U"\b";		// 空でも壊れない
	step( in );
	EXPECT_EQ( "", value );
}

TEST_F( ActionTest, 入力欄は最大の文字数を超えず制御文字を入れない )
{
	string_t value;
	auto* field = root->add< TextField >( "name", [&] { return value; }, [&] ( const string_t& v ) { value = v; } );
	field->set_max_length( 3 );
	ready();

	UIInput in;
	in.text = U"日本語です\r\t";
	step( in );

	EXPECT_EQ( "日本語", value );
}

TEST_F( ActionTest, 入力欄がフォーカス中はQとEでタブを切り替えず文字になる )
{
	auto* catcher = root->add< ActionCatcher >( ui_action::TAB_NEXT );
	string_t value;
	catcher->add< TextField >( "name", [&] { return value; }, [&] ( const string_t& v ) { value = v; } );
	ready();

	UIInput in;
	in.tab_next = true;		// E キー
	in.text = U"e";
	step( in );

	EXPECT_EQ( 0, catcher->count );
	EXPECT_EQ( "e", value );
}

TEST_F( ActionTest, 層を積むと押している部品に捕捉の解除が届く )
{
	auto* button = root->add< CountingButton >( "ok", nullptr );
	ready();

	// ボタンの上で押したまま
	UIInput press;
	press.pointer_x = button->rect().center_x();
	press.pointer_y = button->rect().center_y();
	press.pointer_pressed = true;
	press.pointer_down = true;
	step( press );

	DialogSpec spec;
	spec.buttons.push_back( { "ok", nullptr } );
	show_dialog( screen, std::move( spec ) );
	step();

	EXPECT_EQ( 1, button->lost );
}

TEST_F( ActionTest, ダイアログは既定のボタンに最初のフォーカスを置く )
{
	root->add< Button >( "back", nullptr );
	ready();

	bool yes_pressed = false;
	bool no_pressed = false;

	DialogSpec spec;
	spec.buttons.push_back( { "yes", [&] { yes_pressed = true; } } );
	spec.buttons.push_back( { "no", [&] { no_pressed = true; } } );
	spec.default_index = 1;
	show_dialog( screen, std::move( spec ) );
	step();
	step();		// 開いた直後の入力抑止を抜ける

	// 決定すると、最初にフォーカスされた既定のボタン（no）が押される
	confirm();

	EXPECT_FALSE( yes_pressed );
	EXPECT_TRUE( no_pressed );
}

TEST( TextFieldTest, UTF8の文字数と末尾の1文字の削除 )
{
	EXPECT_EQ( 3u, TextField::count_codepoints( "aあ𠮷" ) );
	EXPECT_EQ( "aあ", TextField::erase_last_codepoint( "aあ𠮷" ) );
	EXPECT_EQ( "", TextField::erase_last_codepoint( "" ) );
	EXPECT_EQ( "𠮷", TextField::to_utf8( U'𠮷' ) );
	EXPECT_EQ( "", TextField::to_utf8( 0xD800 ) );		// サロゲートは文字ではない
}

// ---- Grid と層の属性（設計上の問題 B8・B7） ----

TEST_F( ActionTest, Gridは上下左右の隣のセルへ移り端では祖先へ抜ける )
{
	// 3 列 × 2 行（最後の行は 2 個）の下にボタン
	auto* grid = root->add< Grid >( 3 );
	std::vector< Button* > cells;

	for ( int i = 0; i < 5; i++ )
	{
		cells.push_back( grid->add< Button >( "cell" + std::to_string( i ), nullptr ) );
	}

	auto* below = root->add< Button >( "below", nullptr );
	ready();

	ASSERT_TRUE( cells[ 0 ]->is_focused() );

	press_nav( NavDir::RIGHT );
	EXPECT_TRUE( cells[ 1 ]->is_focused() );

	press_nav( NavDir::DOWN );
	EXPECT_TRUE( cells[ 4 ]->is_focused() );

	press_nav( NavDir::RIGHT );		// 行の端（右隣は無い）: 祖先（縦の Stack）は左右の軸ではないので動かない
	EXPECT_TRUE( cells[ 4 ]->is_focused() );

	press_nav( NavDir::UP );
	EXPECT_TRUE( cells[ 1 ]->is_focused() );

	press_nav( NavDir::RIGHT );
	press_nav( NavDir::DOWN );		// 3 列目の下は空 : 下の行の最後のセルへ
	EXPECT_TRUE( cells[ 4 ]->is_focused() );

	press_nav( NavDir::DOWN );		// 最後の行の下 : Grid から抜けて下のボタンへ
	EXPECT_TRUE( below->is_focused() );

	// セルは等分の幅で並ぶ
	EXPECT_FLOAT_EQ( cells[ 0 ]->rect().w, cells[ 2 ]->rect().w );
	EXPECT_FLOAT_EQ( cells[ 0 ]->rect().y, cells[ 2 ]->rect().y );
	EXPECT_LT( cells[ 0 ]->rect().y, cells[ 3 ]->rect().y );
}

TEST_F( ActionTest, 表示だけの層は入力を奪わない )
{
	bool pressed = false;
	root->add< Button >( "ok", [&] { pressed = true; } );
	ready();

	// HUD のような表示だけの層を積む
	auto hud = std::make_unique< Stack >( Axis::VERTICAL );
	hud->add< Button >( "hud", nullptr );
	screen.push_layer( std::move( hud ), nullptr, false, false );
	step();

	EXPECT_EQ( 2u, screen.get_layer_count() );

	// 決定は下の層のボタンに届く
	confirm();
	EXPECT_TRUE( pressed );
}

// ---- 文字列キーと寸法（設計上の問題 B4・B12） ----

TEST( StringTableTest, 未登録のキーを記録し値で返す )
{
	StringTable strings;
	strings.set( "ok", "OK" );

	const auto& missing_value = strings.get( "no.such.key" );		// 値で返すので、受けた参照が壊れない
	EXPECT_EQ( "no.such.key", missing_value );
	EXPECT_EQ( "OK", strings.get( "ok" ) );

	ASSERT_EQ( 1u, strings.get_missing_keys().size() );
	EXPECT_EQ( "no.such.key", *strings.get_missing_keys().begin() );
}

TEST_F( ActionTest, そのまま表示する選択肢は翻訳しない )
{
	strings.set( "1920 x 1080", "翻訳されてしまった" );		// たまたま同じキーがあっても

	int index = 0;
	auto* choice = root->add< Choice >( "resolution", std::vector< string_t >{ "1920 x 1080" }, [&] { return index; }, [&] ( int i ) { index = i; } );
	choice->set_options_literal( true );
	ready();

	render();
	EXPECT_TRUE( renderer.has_text( "1920 x 1080" ) );
	EXPECT_FALSE( renderer.has_text( "翻訳されてしまった" ) );
}

TEST_F( ActionTest, 文字を大きくすると行の高さも文字に合わせて広がる )
{
	auto* button = root->add< Button >( "ok", nullptr );
	ready();
	const float_t normal_height = button->rect().h;

	// 文字を 2 倍に : 行の高さは「文字の行の高さ＋余白」より小さくならない
	theme.font_size_normal *= 2.f;
	screen.invalidate_layout();
	step();

	EXPECT_GT( button->rect().h, normal_height );
	EXPECT_GE( button->rect().h, renderer.measure_line_height( theme.font_size_normal ) + theme.row_text_padding );
}

TEST( TextFitTest, 幅に収まらない文字は小さくしそれでも収まらなければ末尾を省略する )
{
	ui_test::MockRenderer r;		// 文字幅 = 文字数 × font_size × 0.5

	// 収まる : そのまま
	string_t text = "abcd";
	core::graphics::TextStyle style;
	style.font_size = 40.f;
	fit_text_to_width( r, 100.f, text, style );
	EXPECT_EQ( "abcd", text );
	EXPECT_FLOAT_EQ( 40.f, style.font_size );

	// 少しはみ出す（幅 100 に対して 120）: 小さくして収める
	text = "abcdef";
	style.font_size = 40.f;
	fit_text_to_width( r, 100.f, text, style );
	EXPECT_EQ( "abcdef", text );
	EXPECT_LE( r.measure_text_width( text.c_str(), style.font_size ), 100.f + 1e-3f );

	// 大きくはみ出す : 下限まで小さくして、末尾を省略する
	text = "グラフィックスとオーディオの設定";
	style.font_size = 40.f;
	fit_text_to_width( r, 100.f, text, style );
	EXPECT_FLOAT_EQ( 40.f * MIN_TEXT_FIT_SCALE, style.font_size );
	EXPECT_LE( r.measure_text_width( text.c_str(), style.font_size ), 100.f + 1e-3f );
	EXPECT_EQ( "…", text.substr( text.size() - string_t( "…" ).size() ) );
}

TEST( ThemeTest, 文字の大きさの倍率はフォントサイズだけに掛かる )
{
	Theme base;
	const Theme scaled = scale_theme_text( base, 1.5f );

	EXPECT_FLOAT_EQ( base.font_size_normal * 1.5f, scaled.font_size_normal );
	EXPECT_FLOAT_EQ( base.font_size_title * 1.5f, scaled.font_size_title );
	EXPECT_FLOAT_EQ( base.row_height, scaled.row_height );
}

// ---- シーン遷移の要求（設計上の問題 B22） ----

TEST( SceneRequestTest, シーンが出す文字列を遷移の要求に変える )
{
	using blue_sky::SceneRequest;
	using blue_sky::parse_scene_request;

	EXPECT_EQ( SceneRequest::Type::NONE, parse_scene_request( "", true ).type );
	EXPECT_EQ( SceneRequest::Type::POP_OVERLAY, parse_scene_request( "pop", true ).type );
	EXPECT_EQ( SceneRequest::Type::NONE, parse_scene_request( "pop", false ).type );		// ベースは pop できない
	EXPECT_EQ( SceneRequest::Type::QUIT, parse_scene_request( "quit", false ).type );

	EXPECT_EQ( ( SceneRequest{ SceneRequest::Type::PUSH_OVERLAY, "options" } ), parse_scene_request( "push:options", true ) );
	EXPECT_EQ( ( SceneRequest{ SceneRequest::Type::REPLACE_BASE, "stage_select" } ), parse_scene_request( "base:stage_select", true ) );
	EXPECT_EQ( ( SceneRequest{ SceneRequest::Type::REPLACE_OVERLAY, "options" } ), parse_scene_request( "options", true ) );
	EXPECT_EQ( ( SceneRequest{ SceneRequest::Type::REPLACE_BASE, "title" } ), parse_scene_request( "title", false ) );
}
