#include "OptionsScene.h"

#include <blue_sky/GameMain.h>
#include <blue_sky/GameSettings.h>
#include <blue_sky/Input.h>
#include <blue_sky/KeyNames.h>
#include <blue_sky/graphics/GraphicsManager.h>

#include <core/setting/SettingStore.h>
#include <core/ui/Button.h>
#include <core/ui/Choice.h>
#include <core/ui/Dialog.h>
#include <core/ui/KeyBind.h>
#include <core/ui/Label.h>
#include <core/ui/Panel.h>
#include <core/ui/ScrollView.h>
#include <core/ui/Slider.h>
#include <core/ui/Stack.h>
#include <core/ui/Theme.h>
#include <core/ui/StringTable.h>
#include <core/ui/TabView.h>
#include <core/ui/Toggle.h>

#include <game/Config.h>

#include <algorithm>
#include <cmath>

namespace blue_sky
{

namespace gs = game_settings;
using namespace core::ui;
using core::setting::SettingStore;

namespace
{

/// 縦の Stack を ScrollView に入れたページを作る
Stack* add_scroll_page( TabView* tabs, const char_t* title_key )
{
	auto* scroll = tabs->add_page< ScrollView >( title_key );
	return scroll->add< Stack >( Axis::VERTICAL );
}

/// 0..1 を「50%」の形に（書式は文字列テーブルの format.percent。言語によって「50 %」「%50」などと変えられる）
string_t format_percent( float_t v )
{
	const auto& strings = GameMain::get_instance()->get_ui_strings();
	const string_t number = std::to_string( static_cast< int >( std::lround( v * 100.f ) ) );
	return strings.has( "format.percent" ) ? strings.format( "format.percent", { number } ) : number + "%";
}

string_t format_int( float_t v )
{
	return std::to_string( static_cast< int >( std::lround( v ) ) );
}

string_t format_one_decimal( float_t v )
{
	const int tenth = static_cast< int >( std::lround( v * 10.f ) );
	return std::to_string( tenth / 10 ) + "." + std::to_string( std::abs( tenth % 10 ) );
}

/**
 * 設定の項目からスライダーを作る（範囲・刻み・ラベル・説明はストアの登録から取る）
 *
 * 以前は範囲を画面側にも書いていて、ストアの登録と食い違うおそれがあった。
 *
 * @param also 値を書いたあとに追加で行うこと（X / Y の感度をそろえるなど）
 */
Slider* add_setting_slider( Stack* page, SettingStore* store, const core::setting::SettingKey< float_t >& key,
	std::function< string_t( float_t ) > formatter, std::function< void( float_t ) > also = {} )
{
	SettingStore::Range range;
	store->get_range( key, range );
	const auto info = store->get_info( key );

	auto* slider = page->add< Slider >( info.label_key,
		[store, key] { return store->get( key ); },
		[store, key, also] ( float_t v )
		{
			store->set( key, v );

			if ( also )
			{
				also( v );
			}
		},
		range.min, range.max, range.step );

	slider->set_formatter( std::move( formatter ) );
	slider->set_description_key( info.description_key );
	slider->set_on_commit( [store, key] { store->apply_pending( key ); } );		// ON_RELEASE の項目はここで反映
	return slider;
}

/// 設定の項目からトグルを作る（ラベル・説明はストアの登録から取る）
Toggle* add_setting_toggle( Stack* page, SettingStore* store, const core::setting::SettingKey< bool >& key )
{
	const auto info = store->get_info( key );

	auto* toggle = page->add< Toggle >( info.label_key,
		[store, key] { return store->get( key ); },
		[store, key] ( bool v ) { store->set( key, v ); } );

	toggle->set_description_key( info.description_key );
	return toggle;
}

} // namespace

OptionsScene::OptionsScene()
{
	// 適用・確認・ロールバック・保存の流れ（テストできるよう、依存を渡して切り離してある）
	OptionsController::Dependencies deps;
	deps.store = & get_settings();
	deps.screen = & screen_;
	deps.display_keys = gs::get_display_keys();
	deps.save = services_.save_settings;
	deps.close = [this] { close(); };
	deps.now = [this] { return get_total_elapsed_time(); };
	deps.confirm_display_message = [this] ( int remaining )
	{
		return services_.strings.format( "options.confirm_display", { std::to_string( remaining ) } );
	};
	controller_ = std::make_unique< OptionsController >( std::move( deps ) );

	// 解像度の選択肢（DXGI の一覧から重複を除く）
	for ( const auto& m : get_graphics_manager()->get_available_display_modes() )
	{
		if ( find_resolution_index( m.width, m.height ) < 0 )
		{
			resolutions_.push_back( Resolution{ m.width, m.height } );
		}
	}

	auto panel = std::make_unique< Panel >( "options.title" );
	auto* body = panel->add< Stack >( Axis::VERTICAL );
	body->set_spacing( services_.theme.section_spacing );

	auto* tabs = body->add< TabView >();
	build_graphics_page( add_scroll_page( tabs, "options.tab.graphics" ) );
	build_audio_page( add_scroll_page( tabs, "options.tab.audio" ) );
	build_controls_page( add_scroll_page( tabs, "options.tab.controls" ) );
	body->set_flex( tabs );

	// 下のボタン行
	auto* buttons = body->add< Stack >( Axis::HORIZONTAL );
	SettingStore* store = & get_settings();

	auto* apply_button = buttons->add< Button >( "options.apply", [this] { controller_->apply( false ); } );
	apply_button->set_enabled_if( [store] { return store->has_pending_deferred(); } );
	apply_button->set_description_key( "options.apply.description" );

	auto* defaults_button = buttons->add< Button >( "options.defaults", [this] { controller_->reset_to_defaults(); } );
	defaults_button->set_description_key( "options.defaults.description" );

	buttons->add< Button >( "options.back", [this] { controller_->back(); } );

	screen_.set_root( std::move( panel ), [this] { controller_->back(); } );
}

int OptionsScene::find_resolution_index( int w, int h ) const
{
	for ( size_t i = 0; i < resolutions_.size(); i++ )
	{
		if ( resolutions_[ i ].width == w && resolutions_[ i ].height == h )
		{
			return static_cast< int >( i );
		}
	}

	return -1;
}

void OptionsScene::build_graphics_page( Stack* page )
{
	SettingStore* store = & get_settings();

	std::vector< string_t > options;

	for ( const auto& r : resolutions_ )
	{
		options.push_back( std::to_string( r.width ) + " x " + std::to_string( r.height ) );
	}

	auto* resolution = page->add< Choice >( "options.resolution", options,
		[this, store] { return find_resolution_index( store->get( gs::SCREEN_WIDTH ), store->get( gs::SCREEN_HEIGHT ) ); },
		[this, store] ( int i )
		{
			store->set( gs::SCREEN_WIDTH, resolutions_[ i ].width );
			store->set( gs::SCREEN_HEIGHT, resolutions_[ i ].height );
		} );

	// 現在の解像度が一覧に無いときの表示（勝手に一覧の値へ書き換えない）
	resolution->set_custom_text( [this, store]
	{
		const auto& strings = services_.strings;
		return strings.format( "options.resolution.custom", { std::to_string( store->get( gs::SCREEN_WIDTH ) ) + " x " + std::to_string( store->get( gs::SCREEN_HEIGHT ) ) } );
	} );
	resolution->set_options_literal( true );		// "1920 x 1080" はキーではない
	resolution->set_description_key( "options.resolution.description" );

	add_setting_toggle( page, store, gs::FULL_SCREEN );
	add_setting_slider( page, store, gs::CAMERA_FOV, & format_int );
	add_setting_toggle( page, store, gs::DISPLAY_FPS );
	add_setting_toggle( page, store, gs::VSYNC );
	add_setting_slider( page, store, gs::UI_TEXT_SCALE, & format_percent );
}

void OptionsScene::build_audio_page( Stack* page )
{
	SettingStore* store = & get_settings();

	add_setting_slider( page, store, gs::AUDIO_VOLUME, & format_percent );
	add_setting_toggle( page, store, gs::AUDIO_MUTE );
}

void OptionsScene::build_controls_page( Stack* page )
{
	SettingStore* store = & get_settings();

	// 感度は X / Y を同じ値で設定する
	add_setting_slider( page, store, gs::MOUSE_X_SENSITIVITY, & format_one_decimal,
		[store] ( float_t v ) { store->set( gs::MOUSE_Y_SENSITIVITY, v ); } );

	page->add< Label >( "options.key_bindings", Label::Style::HEADER );

	// キーの表示名 : 文字列テーブルの key.<名前> があればそれ、無ければ名前そのもの
	auto key_display_name = [this] ( int code ) -> string_t
	{
		if ( key_names::is_pad_code( code ) )
		{
			return services_.strings.format( "key.pad", { std::to_string( key_names::pad_button_index( code ) + 1 ) } );
		}

		const string_t name = key_names::name_from_code( code );
		const auto& strings = services_.strings;
		const string_t table_key = "key." + name;
		return strings.has( table_key ) ? strings.get( table_key ) : name;
	};

	for ( const auto& b : gs::get_bindable_buttons() )
	{
		const string_t key = Input::get_key_config_name( b.button );

		page->add< KeyBind >( b.label_key,
			[store, key] { return key_names::code_from_name( gs::get_primary_key( store->get< string_t >( key ) ) ); },
			[store, key] ( int code ) { store->set( key, gs::replace_primary_key( store->get< string_t >( key ), key_names::name_from_code( code ) ) ); },
			key_display_name )
			->set_description_key( "options.key.description" );
	}
}

} // namespace blue_sky
