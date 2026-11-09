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

/// 0..1 を「50%」の形に
string_t format_percent( float_t v )
{
	return std::to_string( static_cast< int >( std::lround( v * 100.f ) ) ) + "%";
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
	body->set_spacing( GameMain::get_instance()->get_ui_theme().section_spacing );

	auto* tabs = body->add< TabView >();
	build_graphics_page( add_scroll_page( tabs, "options.tab.graphics" ) );
	build_audio_page( add_scroll_page( tabs, "options.tab.audio" ) );
	build_controls_page( add_scroll_page( tabs, "options.tab.controls" ) );
	body->set_flex( tabs );

	// 下のボタン行
	auto* buttons = body->add< Stack >( Axis::HORIZONTAL );
	SettingStore* store = & get_settings();

	auto* apply_button = buttons->add< Button >( "options.apply", [this] { apply( false ); } );
	apply_button->set_enabled_if( [store] { return store->has_pending_deferred(); } );
	apply_button->set_description_key( "options.apply.description" );

	auto* defaults_button = buttons->add< Button >( "options.defaults", [this] { reset_to_defaults(); } );
	defaults_button->set_description_key( "options.defaults.description" );

	buttons->add< Button >( "options.back", [this] { back(); } );

	screen_.set_root( std::move( panel ), [this] { back(); } );
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
	resolution->set_custom_text( [store]
	{
		const auto& strings = GameMain::get_instance()->get_ui_strings();
		return strings.format( "options.resolution.custom", { std::to_string( store->get( gs::SCREEN_WIDTH ) ) + " x " + std::to_string( store->get( gs::SCREEN_HEIGHT ) ) } );
	} );
	resolution->set_options_literal( true );		// "1920 x 1080" はキーではない
	resolution->set_description_key( "options.resolution.description" );

	add_setting_toggle( page, store, gs::FULL_SCREEN );
	add_setting_slider( page, store, gs::CAMERA_FOV, & format_int );
	add_setting_toggle( page, store, gs::DISPLAY_FPS );
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
	auto key_display_name = [] ( int code ) -> string_t
	{
		const string_t name = key_names::name_from_code( code );
		const auto& strings = GameMain::get_instance()->get_ui_strings();
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

void OptionsScene::save()
{
	GameMain::get_instance()->save_settings();
}

void OptionsScene::finish_and_close()
{
	get_settings().commit_immediate();
	save();
	close();
}

void OptionsScene::apply( bool close_after )
{
	SettingStore& store = get_settings();

	const auto snapshot = store.snapshot();
	const bool display_changed = store.apply_deferred();

	store.commit_immediate();

	// 表示が変わったときは、維持が選ばれるまで表示系の項目は元の値で保存する
	// （確認の途中で強制終了・クラッシュしても、次回は映っていた設定で起動する）
	if ( display_changed )
	{
		store.begin_trial( snapshot, gs::get_display_keys() );
	}

	save();

	if ( ! display_changed )
	{
		if ( close_after )
		{
			close();
		}

		return;
	}

	// 表示が変わった : 時間内に「維持」しなければ元に戻す（映らなくなっても自動で復旧する）
	constexpr float_t TIMEOUT = 15.f;
	const float_t start = get_total_elapsed_time();

	DialogSpec spec;
	spec.message_provider = [this, start]
	{
		const int remaining = std::max( 0, static_cast< int >( std::ceil( TIMEOUT - ( get_total_elapsed_time() - start ) ) ) );
		return GameMain::get_instance()->get_ui_strings().format( "options.confirm_display", { std::to_string( remaining ) } );
	};
	spec.buttons =
	{
		{ "options.keep", [this, close_after]
			{
				get_settings().end_trial();
				save();

				if ( close_after )
				{
					close();
				}
			} },
		{ "options.revert", [this, snapshot]
			{
				get_settings().rollback( snapshot, gs::get_display_keys() );
				save();
			} },
	};
	spec.cancel_index = 1;
	spec.default_index = 0;
	spec.timeout_sec = TIMEOUT;

	show_dialog( screen_, std::move( spec ) );
}

void OptionsScene::back()
{
	if ( ! get_settings().has_pending_deferred() )
	{
		finish_and_close();
		return;
	}

	DialogSpec spec;
	spec.message = "options.confirm_unapplied";
	spec.buttons =
	{
		{ "options.apply_and_back", [this] { apply( true ); } },
		{ "options.discard_and_back", [this] { get_settings().revert_deferred(); finish_and_close(); } },
		{ "common.cancel", nullptr },
	};
	spec.cancel_index = 2;
	spec.default_index = 0;

	show_dialog( screen_, std::move( spec ) );
}

void OptionsScene::reset_to_defaults()
{
	DialogSpec spec;
	spec.message = "options.confirm_defaults";
	spec.buttons =
	{
		{ "common.yes", [this] { get_settings().reset_to_defaults(); } },
		{ "common.no", nullptr },
	};
	spec.cancel_index = 1;
	spec.default_index = 1;

	show_dialog( screen_, std::move( spec ) );
}

} // namespace blue_sky
